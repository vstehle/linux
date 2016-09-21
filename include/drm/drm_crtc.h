/*
 * Copyright © 2006 Keith Packard
 * Copyright © 2007-2008 Dave Airlie
 * Copyright © 2007-2008 Intel Corporation
 *   Jesse Barnes <jesse.barnes@intel.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __DRM_CRTC_H__
#define __DRM_CRTC_H__

#include <linux/i2c.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/idr.h>
#include <linux/fb.h>
#include <linux/hdmi.h>
#include <linux/media-bus-format.h>
#include <uapi/drm/drm_mode.h>
#include <uapi/drm/drm_fourcc.h>
#include <drm/drm_modeset_lock.h>
#include <drm/drm_rect.h>
#include <drm/drm_mode_object.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_modes.h>
#include <drm/drm_connector.h>
#include <drm/drm_encoder.h>
#include <drm/drm_property.h>
#include <drm/drm_edid.h>
#include <drm/drm_plane.h>
#include <drm/drm_blend.h>
#include <drm/drm_color_mgmt.h>

struct drm_device;
struct drm_mode_set;
struct drm_file;
struct drm_clip_rect;
struct device_node;
struct fence;
struct edid;

static inline int64_t U642I64(uint64_t val)
{
	return (int64_t)*((int64_t *)&val);
}
static inline uint64_t I642U64(int64_t val)
{
	return (uint64_t)*((uint64_t *)&val);
}

/* data corresponds to displayid vend/prod/serial */
struct drm_tile_group {
	struct kref refcount;
	struct drm_device *dev;
	int id;
	u8 group_data[8];
};

struct drm_crtc;
struct drm_encoder;
struct drm_pending_vblank_event;
struct drm_plane;
struct drm_bridge;
struct drm_atomic_state;

struct drm_crtc_helper_funcs;
struct drm_encoder_helper_funcs;
struct drm_plane_helper_funcs;

/**
 * struct drm_crtc_state - mutable CRTC state
 * @crtc: backpointer to the CRTC
 * @enable: whether the CRTC should be enabled, gates all other state
 * @active: whether the CRTC is actively displaying (used for DPMS)
 * @planes_changed: planes on this crtc are updated
 * @mode_changed: crtc_state->mode or crtc_state->enable has been changed
 * @active_changed: crtc_state->active has been toggled.
 * @connectors_changed: connectors to this crtc have been updated
 * @zpos_changed: zpos values of planes on this crtc have been updated
 * @color_mgmt_changed: color management properties have changed (degamma or
 *	gamma LUT or CSC matrix)
 * @plane_mask: bitmask of (1 << drm_plane_index(plane)) of attached planes
 * @connector_mask: bitmask of (1 << drm_connector_index(connector)) of attached connectors
 * @encoder_mask: bitmask of (1 << drm_encoder_index(encoder)) of attached encoders
 * @last_vblank_count: for helpers and drivers to capture the vblank of the
 * 	update to ensure framebuffer cleanup isn't done too early
 * @adjusted_mode: for use by helpers and drivers to compute adjusted mode timings
 * @mode: current mode timings
 * @degamma_lut: Lookup table for converting framebuffer pixel data
 *	before apply the conversion matrix
 * @ctm: Transformation matrix
 * @gamma_lut: Lookup table for converting pixel data after the
 *	conversion matrix
 * @event: optional pointer to a DRM event to signal upon completion of the
 * 	state update
 * @state: backpointer to global drm_atomic_state
 *
 * Note that the distinction between @enable and @active is rather subtile:
 * Flipping @active while @enable is set without changing anything else may
 * never return in a failure from the ->atomic_check callback. Userspace assumes
 * that a DPMS On will always succeed. In other words: @enable controls resource
 * assignment, @active controls the actual hardware state.
 */
struct drm_crtc_state {
	struct drm_crtc *crtc;

	bool enable;
	bool active;

	/* computed state bits used by helpers and drivers */
	bool planes_changed : 1;
	bool mode_changed : 1;
	bool active_changed : 1;
	bool connectors_changed : 1;
	bool zpos_changed : 1;
	bool color_mgmt_changed : 1;

	/* attached planes bitmask:
	 * WARNING: transitional helpers do not maintain plane_mask so
	 * drivers not converted over to atomic helpers should not rely
	 * on plane_mask being accurate!
	 */
	u32 plane_mask;

	u32 connector_mask;
	u32 encoder_mask;

	/* last_vblank_count: for vblank waits before cleanup */
	u32 last_vblank_count;

	/* adjusted_mode: for use by helpers and drivers */
	struct drm_display_mode adjusted_mode;

	struct drm_display_mode mode;

	/* blob property to expose current mode to atomic userspace */
	struct drm_property_blob *mode_blob;

	/* blob property to expose color management to userspace */
	struct drm_property_blob *degamma_lut;
	struct drm_property_blob *ctm;
	struct drm_property_blob *gamma_lut;

	struct drm_pending_vblank_event *event;

	struct drm_atomic_state *state;
};

/**
 * struct drm_crtc_funcs - control CRTCs for a given device
 *
 * The drm_crtc_funcs structure is the central CRTC management structure
 * in the DRM.  Each CRTC controls one or more connectors (note that the name
 * CRTC is simply historical, a CRTC may control LVDS, VGA, DVI, TV out, etc.
 * connectors, not just CRTs).
 *
 * Each driver is responsible for filling out this structure at startup time,
 * in addition to providing other modesetting features, like i2c and DDC
 * bus accessors.
 */
struct drm_crtc_funcs {
	/**
	 * @reset:
	 *
	 * Reset CRTC hardware and software state to off. This function isn't
	 * called by the core directly, only through drm_mode_config_reset().
	 * It's not a helper hook only for historical reasons.
	 *
	 * Atomic drivers can use drm_atomic_helper_crtc_reset() to reset
	 * atomic state using this hook.
	 */
	void (*reset)(struct drm_crtc *crtc);

	/**
	 * @cursor_set:
	 *
	 * Update the cursor image. The cursor position is relative to the CRTC
	 * and can be partially or fully outside of the visible area.
	 *
	 * Note that contrary to all other KMS functions the legacy cursor entry
	 * points don't take a framebuffer object, but instead take directly a
	 * raw buffer object id from the driver's buffer manager (which is
	 * either GEM or TTM for current drivers).
	 *
	 * This entry point is deprecated, drivers should instead implement
	 * universal plane support and register a proper cursor plane using
	 * drm_crtc_init_with_planes().
	 *
	 * This callback is optional
	 *
	 * RETURNS:
	 *
	 * 0 on success or a negative error code on failure.
	 */
	int (*cursor_set)(struct drm_crtc *crtc, struct drm_file *file_priv,
			  uint32_t handle, uint32_t width, uint32_t height);

	/**
	 * @cursor_set2:
	 *
	 * Update the cursor image, including hotspot information. The hotspot
	 * must not affect the cursor position in CRTC coordinates, but is only
	 * meant as a hint for virtualized display hardware to coordinate the
	 * guests and hosts cursor position. The cursor hotspot is relative to
	 * the cursor image. Otherwise this works exactly like @cursor_set.
	 *
	 * This entry point is deprecated, drivers should instead implement
	 * universal plane support and register a proper cursor plane using
	 * drm_crtc_init_with_planes().
	 *
	 * This callback is optional.
	 *
	 * RETURNS:
	 *
	 * 0 on success or a negative error code on failure.
	 */
	int (*cursor_set2)(struct drm_crtc *crtc, struct drm_file *file_priv,
			   uint32_t handle, uint32_t width, uint32_t height,
			   int32_t hot_x, int32_t hot_y);

	/**
	 * @cursor_move:
	 *
	 * Update the cursor position. The cursor does not need to be visible
	 * when this hook is called.
	 *
	 * This entry point is deprecated, drivers should instead implement
	 * universal plane support and register a proper cursor plane using
	 * drm_crtc_init_with_planes().
	 *
	 * This callback is optional.
	 *
	 * RETURNS:
	 *
	 * 0 on success or a negative error code on failure.
	 */
	int (*cursor_move)(struct drm_crtc *crtc, int x, int y);

	/**
	 * @gamma_set:
	 *
	 * Set gamma on the CRTC.
	 *
	 * This callback is optional.
	 *
	 * NOTE:
	 *
	 * Drivers that support gamma tables and also fbdev emulation through
	 * the provided helper library need to take care to fill out the gamma
	 * hooks for both. Currently there's a bit an unfortunate duplication
	 * going on, which should eventually be unified to just one set of
	 * hooks.
	 */
	int (*gamma_set)(struct drm_crtc *crtc, u16 *r, u16 *g, u16 *b,
			 uint32_t size);

	/**
	 * @destroy:
	 *
	 * Clean up plane resources. This is only called at driver unload time
	 * through drm_mode_config_cleanup() since a CRTC cannot be hotplugged
	 * in DRM.
	 */
	void (*destroy)(struct drm_crtc *crtc);

	/**
	 * @set_config:
	 *
	 * This is the main legacy entry point to change the modeset state on a
	 * CRTC. All the details of the desired configuration are passed in a
	 * struct &drm_mode_set - see there for details.
	 *
	 * Drivers implementing atomic modeset should use
	 * drm_atomic_helper_set_config() to implement this hook.
	 *
	 * RETURNS:
	 *
	 * 0 on success or a negative error code on failure.
	 */
	int (*set_config)(struct drm_mode_set *set);

	/**
	 * @page_flip:
	 *
	 * Legacy entry point to schedule a flip to the given framebuffer.
	 *
	 * Page flipping is a synchronization mechanism that replaces the frame
	 * buffer being scanned out by the CRTC with a new frame buffer during
	 * vertical blanking, avoiding tearing (except when requested otherwise
	 * through the DRM_MODE_PAGE_FLIP_ASYNC flag). When an application
	 * requests a page flip the DRM core verifies that the new frame buffer
	 * is large enough to be scanned out by the CRTC in the currently
	 * configured mode and then calls the CRTC ->page_flip() operation with a
	 * pointer to the new frame buffer.
	 *
	 * The driver must wait for any pending rendering to the new framebuffer
	 * to complete before executing the flip. It should also wait for any
	 * pending rendering from other drivers if the underlying buffer is a
	 * shared dma-buf.
	 *
	 * An application can request to be notified when the page flip has
	 * completed. The drm core will supply a struct &drm_event in the event
	 * parameter in this case. This can be handled by the
	 * drm_crtc_send_vblank_event() function, which the driver should call on
	 * the provided event upon completion of the flip. Note that if
	 * the driver supports vblank signalling and timestamping the vblank
	 * counters and timestamps must agree with the ones returned from page
	 * flip events. With the current vblank helper infrastructure this can
	 * be achieved by holding a vblank reference while the page flip is
	 * pending, acquired through drm_crtc_vblank_get() and released with
	 * drm_crtc_vblank_put(). Drivers are free to implement their own vblank
	 * counter and timestamp tracking though, e.g. if they have accurate
	 * timestamp registers in hardware.
	 *
	 * FIXME:
	 *
	 * Up to that point drivers need to manage events themselves and can use
	 * even->base.list freely for that. Specifically they need to ensure
	 * that they don't send out page flip (or vblank) events for which the
	 * corresponding drm file has been closed already. The drm core
	 * unfortunately does not (yet) take care of that. Therefore drivers
	 * currently must clean up and release pending events in their
	 * ->preclose driver function.
	 *
	 * This callback is optional.
	 *
	 * NOTE:
	 *
	 * Very early versions of the KMS ABI mandated that the driver must
	 * block (but not reject) any rendering to the old framebuffer until the
	 * flip operation has completed and the old framebuffer is no longer
	 * visible. This requirement has been lifted, and userspace is instead
	 * expected to request delivery of an event and wait with recycling old
	 * buffers until such has been received.
	 *
	 * RETURNS:
	 *
	 * 0 on success or a negative error code on failure. Note that if a
	 * ->page_flip() operation is already pending the callback should return
	 * -EBUSY. Pageflips on a disabled CRTC (either by setting a NULL mode
	 * or just runtime disabled through DPMS respectively the new atomic
	 * "ACTIVE" state) should result in an -EINVAL error code. Note that
	 * drm_atomic_helper_page_flip() checks this already for atomic drivers.
	 */
	int (*page_flip)(struct drm_crtc *crtc,
			 struct drm_framebuffer *fb,
			 struct drm_pending_vblank_event *event,
			 uint32_t flags);

	/**
	 * @page_flip_target:
	 *
	 * Same as @page_flip but with an additional parameter specifying the
	 * absolute target vertical blank period (as reported by
	 * drm_crtc_vblank_count()) when the flip should take effect.
	 *
	 * Note that the core code calls drm_crtc_vblank_get before this entry
	 * point, and will call drm_crtc_vblank_put if this entry point returns
	 * any non-0 error code. It's the driver's responsibility to call
	 * drm_crtc_vblank_put after this entry point returns 0, typically when
	 * the flip completes.
	 */
	int (*page_flip_target)(struct drm_crtc *crtc,
				struct drm_framebuffer *fb,
				struct drm_pending_vblank_event *event,
				uint32_t flags, uint32_t target);

	/**
	 * @set_property:
	 *
	 * This is the legacy entry point to update a property attached to the
	 * CRTC.
	 *
	 * Drivers implementing atomic modeset should use
	 * drm_atomic_helper_crtc_set_property() to implement this hook.
	 *
	 * This callback is optional if the driver does not support any legacy
	 * driver-private properties.
	 *
	 * RETURNS:
	 *
	 * 0 on success or a negative error code on failure.
	 */
	int (*set_property)(struct drm_crtc *crtc,
			    struct drm_property *property, uint64_t val);

	/**
	 * @atomic_duplicate_state:
	 *
	 * Duplicate the current atomic state for this CRTC and return it.
	 * The core and helpers gurantee that any atomic state duplicated with
	 * this hook and still owned by the caller (i.e. not transferred to the
	 * driver by calling ->atomic_commit() from struct
	 * &drm_mode_config_funcs) will be cleaned up by calling the
	 * @atomic_destroy_state hook in this structure.
	 *
	 * Atomic drivers which don't subclass struct &drm_crtc should use
	 * drm_atomic_helper_crtc_duplicate_state(). Drivers that subclass the
	 * state structure to extend it with driver-private state should use
	 * __drm_atomic_helper_crtc_duplicate_state() to make sure shared state is
	 * duplicated in a consistent fashion across drivers.
	 *
	 * It is an error to call this hook before crtc->state has been
	 * initialized correctly.
	 *
	 * NOTE:
	 *
	 * If the duplicate state references refcounted resources this hook must
	 * acquire a reference for each of them. The driver must release these
	 * references again in @atomic_destroy_state.
	 *
	 * RETURNS:
	 *
	 * Duplicated atomic state or NULL when the allocation failed.
	 */
	struct drm_crtc_state *(*atomic_duplicate_state)(struct drm_crtc *crtc);

	/**
	 * @atomic_destroy_state:
	 *
	 * Destroy a state duplicated with @atomic_duplicate_state and release
	 * or unreference all resources it references
	 */
	void (*atomic_destroy_state)(struct drm_crtc *crtc,
				     struct drm_crtc_state *state);

	/**
	 * @atomic_set_property:
	 *
	 * Decode a driver-private property value and store the decoded value
	 * into the passed-in state structure. Since the atomic core decodes all
	 * standardized properties (even for extensions beyond the core set of
	 * properties which might not be implemented by all drivers) this
	 * requires drivers to subclass the state structure.
	 *
	 * Such driver-private properties should really only be implemented for
	 * truly hardware/vendor specific state. Instead it is preferred to
	 * standardize atomic extension and decode the properties used to expose
	 * such an extension in the core.
	 *
	 * Do not call this function directly, use
	 * drm_atomic_crtc_set_property() instead.
	 *
	 * This callback is optional if the driver does not support any
	 * driver-private atomic properties.
	 *
	 * NOTE:
	 *
	 * This function is called in the state assembly phase of atomic
	 * modesets, which can be aborted for any reason (including on
	 * userspace's request to just check whether a configuration would be
	 * possible). Drivers MUST NOT touch any persistent state (hardware or
	 * software) or data structures except the passed in @state parameter.
	 *
	 * Also since userspace controls in which order properties are set this
	 * function must not do any input validation (since the state update is
	 * incomplete and hence likely inconsistent). Instead any such input
	 * validation must be done in the various atomic_check callbacks.
	 *
	 * RETURNS:
	 *
	 * 0 if the property has been found, -EINVAL if the property isn't
	 * implemented by the driver (which should never happen, the core only
	 * asks for properties attached to this CRTC). No other validation is
	 * allowed by the driver. The core already checks that the property
	 * value is within the range (integer, valid enum value, ...) the driver
	 * set when registering the property.
	 */
	int (*atomic_set_property)(struct drm_crtc *crtc,
				   struct drm_crtc_state *state,
				   struct drm_property *property,
				   uint64_t val);
	/**
	 * @atomic_get_property:
	 *
	 * Reads out the decoded driver-private property. This is used to
	 * implement the GETCRTC IOCTL.
	 *
	 * Do not call this function directly, use
	 * drm_atomic_crtc_get_property() instead.
	 *
	 * This callback is optional if the driver does not support any
	 * driver-private atomic properties.
	 *
	 * RETURNS:
	 *
	 * 0 on success, -EINVAL if the property isn't implemented by the
	 * driver (which should never happen, the core only asks for
	 * properties attached to this CRTC).
	 */
	int (*atomic_get_property)(struct drm_crtc *crtc,
				   const struct drm_crtc_state *state,
				   struct drm_property *property,
				   uint64_t *val);

	/**
	 * @late_register:
	 *
	 * This optional hook can be used to register additional userspace
	 * interfaces attached to the crtc like debugfs interfaces.
	 * It is called late in the driver load sequence from drm_dev_register().
	 * Everything added from this callback should be unregistered in
	 * the early_unregister callback.
	 *
	 * Returns:
	 *
	 * 0 on success, or a negative error code on failure.
	 */
	int (*late_register)(struct drm_crtc *crtc);

	/**
	 * @early_unregister:
	 *
	 * This optional hook should be used to unregister the additional
	 * userspace interfaces attached to the crtc from
	 * late_unregister(). It is called from drm_dev_unregister(),
	 * early in the driver unload sequence to disable userspace access
	 * before data structures are torndown.
	 */
	void (*early_unregister)(struct drm_crtc *crtc);
};

/**
 * struct drm_crtc - central CRTC control structure
 * @dev: parent DRM device
 * @port: OF node used by drm_of_find_possible_crtcs()
 * @head: list management
 * @mutex: per-CRTC locking
 * @base: base KMS object for ID tracking etc.
 * @primary: primary plane for this CRTC
 * @cursor: cursor plane for this CRTC
 * @cursor_x: current x position of the cursor, used for universal cursor planes
 * @cursor_y: current y position of the cursor, used for universal cursor planes
 * @enabled: is this CRTC enabled?
 * @mode: current mode timings
 * @hwmode: mode timings as programmed to hw regs
 * @x: x position on screen
 * @y: y position on screen
 * @funcs: CRTC control functions
 * @gamma_size: size of gamma ramp
 * @gamma_store: gamma ramp values
 * @helper_private: mid-layer private data
 * @properties: property tracking for this CRTC
 *
 * Each CRTC may have one or more connectors associated with it.  This structure
 * allows the CRTC to be controlled.
 */
struct drm_crtc {
	struct drm_device *dev;
	struct device_node *port;
	struct list_head head;

	char *name;

	/*
	 * crtc mutex
	 *
	 * This provides a read lock for the overall crtc state (mode, dpms
	 * state, ...) and a write lock for everything which can be update
	 * without a full modeset (fb, cursor data, ...)
	 */
	struct drm_modeset_lock mutex;

	struct drm_mode_object base;

	/* primary and cursor planes for CRTC */
	struct drm_plane *primary;
	struct drm_plane *cursor;

	/* position inside the mode_config.list, can be used as a [] idx */
	unsigned index;

	/* position of cursor plane on crtc */
	int cursor_x;
	int cursor_y;

	bool enabled;

	/* Requested mode from modesetting. */
	struct drm_display_mode mode;

	/* Programmed mode in hw, after adjustments for encoders,
	 * crtc, panel scaling etc. Needed for timestamping etc.
	 */
	struct drm_display_mode hwmode;

	int x, y;
	const struct drm_crtc_funcs *funcs;

	/* Legacy FB CRTC gamma size for reporting to userspace */
	uint32_t gamma_size;
	uint16_t *gamma_store;

	/* if you are using the helper */
	const struct drm_crtc_helper_funcs *helper_private;

	struct drm_object_properties properties;

	/**
	 * @state:
	 *
	 * Current atomic state for this CRTC.
	 */
	struct drm_crtc_state *state;

	/**
	 * @commit_list:
	 *
	 * List of &drm_crtc_commit structures tracking pending commits.
	 * Protected by @commit_lock. This list doesn't hold its own full
	 * reference, but burrows it from the ongoing commit. Commit entries
	 * must be removed from this list once the commit is fully completed,
	 * but before it's correspoding &drm_atomic_state gets destroyed.
	 */
	struct list_head commit_list;

	/**
	 * @commit_lock:
	 *
	 * Spinlock to protect @commit_list.
	 */
	spinlock_t commit_lock;

	/**
	 * @acquire_ctx:
	 *
	 * Per-CRTC implicit acquire context used by atomic drivers for legacy
	 * IOCTLs, so that atomic drivers can get at the locking acquire
	 * context.
	 */
	struct drm_modeset_acquire_ctx *acquire_ctx;
};

/**
 * struct drm_bridge_funcs - drm_bridge control functions
 * @attach: Called during drm_bridge_attach
 */
struct drm_bridge_funcs {
	int (*attach)(struct drm_bridge *bridge);

	/**
	 * @mode_fixup:
	 *
	 * This callback is used to validate and adjust a mode. The paramater
	 * mode is the display mode that should be fed to the next element in
	 * the display chain, either the final &drm_connector or the next
	 * &drm_bridge. The parameter adjusted_mode is the input mode the bridge
	 * requires. It can be modified by this callback and does not need to
	 * match mode.
	 *
	 * This is the only hook that allows a bridge to reject a modeset. If
	 * this function passes all other callbacks must succeed for this
	 * configuration.
	 *
	 * NOTE:
	 *
	 * This function is called in the check phase of atomic modesets, which
	 * can be aborted for any reason (including on userspace's request to
	 * just check whether a configuration would be possible). Drivers MUST
	 * NOT touch any persistent state (hardware or software) or data
	 * structures except the passed in @state parameter.
	 *
	 * RETURNS:
	 *
	 * True if an acceptable configuration is possible, false if the modeset
	 * operation should be rejected.
	 */
	bool (*mode_fixup)(struct drm_bridge *bridge,
			   const struct drm_display_mode *mode,
			   struct drm_display_mode *adjusted_mode);
	/**
	 * @disable:
	 *
	 * This callback should disable the bridge. It is called right before
	 * the preceding element in the display pipe is disabled. If the
	 * preceding element is a bridge this means it's called before that
	 * bridge's ->disable() function. If the preceding element is a
	 * &drm_encoder it's called right before the encoder's ->disable(),
	 * ->prepare() or ->dpms() hook from struct &drm_encoder_helper_funcs.
	 *
	 * The bridge can assume that the display pipe (i.e. clocks and timing
	 * signals) feeding it is still running when this callback is called.
	 *
	 * The disable callback is optional.
	 */
	void (*disable)(struct drm_bridge *bridge);

	/**
	 * @post_disable:
	 *
	 * This callback should disable the bridge. It is called right after
	 * the preceding element in the display pipe is disabled. If the
	 * preceding element is a bridge this means it's called after that
	 * bridge's ->post_disable() function. If the preceding element is a
	 * &drm_encoder it's called right after the encoder's ->disable(),
	 * ->prepare() or ->dpms() hook from struct &drm_encoder_helper_funcs.
	 *
	 * The bridge must assume that the display pipe (i.e. clocks and timing
	 * singals) feeding it is no longer running when this callback is
	 * called.
	 *
	 * The post_disable callback is optional.
	 */
	void (*post_disable)(struct drm_bridge *bridge);

	/**
	 * @mode_set:
	 *
	 * This callback should set the given mode on the bridge. It is called
	 * after the ->mode_set() callback for the preceding element in the
	 * display pipeline has been called already. The display pipe (i.e.
	 * clocks and timing signals) is off when this function is called.
	 */
	void (*mode_set)(struct drm_bridge *bridge,
			 struct drm_display_mode *mode,
			 struct drm_display_mode *adjusted_mode);
	/**
	 * @pre_enable:
	 *
	 * This callback should enable the bridge. It is called right before
	 * the preceding element in the display pipe is enabled. If the
	 * preceding element is a bridge this means it's called before that
	 * bridge's ->pre_enable() function. If the preceding element is a
	 * &drm_encoder it's called right before the encoder's ->enable(),
	 * ->commit() or ->dpms() hook from struct &drm_encoder_helper_funcs.
	 *
	 * The display pipe (i.e. clocks and timing signals) feeding this bridge
	 * will not yet be running when this callback is called. The bridge must
	 * not enable the display link feeding the next bridge in the chain (if
	 * there is one) when this callback is called.
	 *
	 * The pre_enable callback is optional.
	 */
	void (*pre_enable)(struct drm_bridge *bridge);

	/**
	 * @enable:
	 *
	 * This callback should enable the bridge. It is called right after
	 * the preceding element in the display pipe is enabled. If the
	 * preceding element is a bridge this means it's called after that
	 * bridge's ->enable() function. If the preceding element is a
	 * &drm_encoder it's called right after the encoder's ->enable(),
	 * ->commit() or ->dpms() hook from struct &drm_encoder_helper_funcs.
	 *
	 * The bridge can assume that the display pipe (i.e. clocks and timing
	 * signals) feeding it is running when this callback is called. This
	 * callback must enable the display link feeding the next bridge in the
	 * chain if there is one.
	 *
	 * The enable callback is optional.
	 */
	void (*enable)(struct drm_bridge *bridge);
};

/**
 * struct drm_bridge - central DRM bridge control structure
 * @dev: DRM device this bridge belongs to
 * @encoder: encoder to which this bridge is connected
 * @next: the next bridge in the encoder chain
 * @of_node: device node pointer to the bridge
 * @list: to keep track of all added bridges
 * @funcs: control functions
 * @driver_private: pointer to the bridge driver's internal context
 */
struct drm_bridge {
	struct drm_device *dev;
	struct drm_encoder *encoder;
	struct drm_bridge *next;
#ifdef CONFIG_OF
	struct device_node *of_node;
#endif
	struct list_head list;

	const struct drm_bridge_funcs *funcs;
	void *driver_private;
};

/** struct drm_mode_set - new values for a CRTC config change
 * @fb: framebuffer to use for new config
 * @crtc: CRTC whose configuration we're about to change
 * @mode: mode timings to use
 * @x: position of this CRTC relative to @fb
 * @y: position of this CRTC relative to @fb
 * @connectors: array of connectors to drive with this CRTC if possible
 * @num_connectors: size of @connectors array
 *
 * Represents a single crtc the connectors that it drives with what mode
 * and from which framebuffer it scans out from.
 *
 * This is used to set modes.
 */
struct drm_mode_set {
	struct drm_framebuffer *fb;
	struct drm_crtc *crtc;
	struct drm_display_mode *mode;

	uint32_t x;
	uint32_t y;

	struct drm_connector **connectors;
	size_t num_connectors;
};

/**
 * struct drm_mode_config_funcs - basic driver provided mode setting functions
 *
 * Some global (i.e. not per-CRTC, connector, etc) mode setting functions that
 * involve drivers.
 */
struct drm_mode_config_funcs {
	/**
	 * @fb_create:
	 *
	 * Create a new framebuffer object. The core does basic checks on the
	 * requested metadata, but most of that is left to the driver. See
	 * struct &drm_mode_fb_cmd2 for details.
	 *
	 * RETURNS:
	 *
	 * A new framebuffer with an initial reference count of 1 or a negative
	 * error code encoded with ERR_PTR().
	 */
	struct drm_framebuffer *(*fb_create)(struct drm_device *dev,
					     struct drm_file *file_priv,
					     const struct drm_mode_fb_cmd2 *mode_cmd);

	/**
	 * @output_poll_changed:
	 *
	 * Callback used by helpers to inform the driver of output configuration
	 * changes.
	 *
	 * Drivers implementing fbdev emulation with the helpers can call
	 * drm_fb_helper_hotplug_changed from this hook to inform the fbdev
	 * helper of output changes.
	 *
	 * FIXME:
	 *
	 * Except that there's no vtable for device-level helper callbacks
	 * there's no reason this is a core function.
	 */
	void (*output_poll_changed)(struct drm_device *dev);

	/**
	 * @atomic_check:
	 *
	 * This is the only hook to validate an atomic modeset update. This
	 * function must reject any modeset and state changes which the hardware
	 * or driver doesn't support. This includes but is of course not limited
	 * to:
	 *
	 *  - Checking that the modes, framebuffers, scaling and placement
	 *    requirements and so on are within the limits of the hardware.
	 *
	 *  - Checking that any hidden shared resources are not oversubscribed.
	 *    This can be shared PLLs, shared lanes, overall memory bandwidth,
	 *    display fifo space (where shared between planes or maybe even
	 *    CRTCs).
	 *
	 *  - Checking that virtualized resources exported to userspace are not
	 *    oversubscribed. For various reasons it can make sense to expose
	 *    more planes, crtcs or encoders than which are physically there. One
	 *    example is dual-pipe operations (which generally should be hidden
	 *    from userspace if when lockstepped in hardware, exposed otherwise),
	 *    where a plane might need 1 hardware plane (if it's just on one
	 *    pipe), 2 hardware planes (when it spans both pipes) or maybe even
	 *    shared a hardware plane with a 2nd plane (if there's a compatible
	 *    plane requested on the area handled by the other pipe).
	 *
	 *  - Check that any transitional state is possible and that if
	 *    requested, the update can indeed be done in the vblank period
	 *    without temporarily disabling some functions.
	 *
	 *  - Check any other constraints the driver or hardware might have.
	 *
	 *  - This callback also needs to correctly fill out the &drm_crtc_state
	 *    in this update to make sure that drm_atomic_crtc_needs_modeset()
	 *    reflects the nature of the possible update and returns true if and
	 *    only if the update cannot be applied without tearing within one
	 *    vblank on that CRTC. The core uses that information to reject
	 *    updates which require a full modeset (i.e. blanking the screen, or
	 *    at least pausing updates for a substantial amount of time) if
	 *    userspace has disallowed that in its request.
	 *
	 *  - The driver also does not need to repeat basic input validation
	 *    like done for the corresponding legacy entry points. The core does
	 *    that before calling this hook.
	 *
	 * See the documentation of @atomic_commit for an exhaustive list of
	 * error conditions which don't have to be checked at the
	 * ->atomic_check() stage?
	 *
	 * See the documentation for struct &drm_atomic_state for how exactly
	 * an atomic modeset update is described.
	 *
	 * Drivers using the atomic helpers can implement this hook using
	 * drm_atomic_helper_check(), or one of the exported sub-functions of
	 * it.
	 *
	 * RETURNS:
	 *
	 * 0 on success or one of the below negative error codes:
	 *
	 *  - -EINVAL, if any of the above constraints are violated.
	 *
	 *  - -EDEADLK, when returned from an attempt to acquire an additional
	 *    &drm_modeset_lock through drm_modeset_lock().
	 *
	 *  - -ENOMEM, if allocating additional state sub-structures failed due
	 *    to lack of memory.
	 *
	 *  - -EINTR, -EAGAIN or -ERESTARTSYS, if the IOCTL should be restarted.
	 *    This can either be due to a pending signal, or because the driver
	 *    needs to completely bail out to recover from an exceptional
	 *    situation like a GPU hang. From a userspace point all errors are
	 *    treated equally.
	 */
	int (*atomic_check)(struct drm_device *dev,
			    struct drm_atomic_state *state);

	/**
	 * @atomic_commit:
	 *
	 * This is the only hook to commit an atomic modeset update. The core
	 * guarantees that @atomic_check has been called successfully before
	 * calling this function, and that nothing has been changed in the
	 * interim.
	 *
	 * See the documentation for struct &drm_atomic_state for how exactly
	 * an atomic modeset update is described.
	 *
	 * Drivers using the atomic helpers can implement this hook using
	 * drm_atomic_helper_commit(), or one of the exported sub-functions of
	 * it.
	 *
	 * Nonblocking commits (as indicated with the nonblock parameter) must
	 * do any preparatory work which might result in an unsuccessful commit
	 * in the context of this callback. The only exceptions are hardware
	 * errors resulting in -EIO. But even in that case the driver must
	 * ensure that the display pipe is at least running, to avoid
	 * compositors crashing when pageflips don't work. Anything else,
	 * specifically committing the update to the hardware, should be done
	 * without blocking the caller. For updates which do not require a
	 * modeset this must be guaranteed.
	 *
	 * The driver must wait for any pending rendering to the new
	 * framebuffers to complete before executing the flip. It should also
	 * wait for any pending rendering from other drivers if the underlying
	 * buffer is a shared dma-buf. Nonblocking commits must not wait for
	 * rendering in the context of this callback.
	 *
	 * An application can request to be notified when the atomic commit has
	 * completed. These events are per-CRTC and can be distinguished by the
	 * CRTC index supplied in &drm_event to userspace.
	 *
	 * The drm core will supply a struct &drm_event in the event
	 * member of each CRTC's &drm_crtc_state structure. This can be handled by the
	 * drm_crtc_send_vblank_event() function, which the driver should call on
	 * the provided event upon completion of the atomic commit. Note that if
	 * the driver supports vblank signalling and timestamping the vblank
	 * counters and timestamps must agree with the ones returned from page
	 * flip events. With the current vblank helper infrastructure this can
	 * be achieved by holding a vblank reference while the page flip is
	 * pending, acquired through drm_crtc_vblank_get() and released with
	 * drm_crtc_vblank_put(). Drivers are free to implement their own vblank
	 * counter and timestamp tracking though, e.g. if they have accurate
	 * timestamp registers in hardware.
	 *
	 * NOTE:
	 *
	 * Drivers are not allowed to shut down any display pipe successfully
	 * enabled through an atomic commit on their own. Doing so can result in
	 * compositors crashing if a page flip is suddenly rejected because the
	 * pipe is off.
	 *
	 * RETURNS:
	 *
	 * 0 on success or one of the below negative error codes:
	 *
	 *  - -EBUSY, if a nonblocking updated is requested and there is
	 *    an earlier updated pending. Drivers are allowed to support a queue
	 *    of outstanding updates, but currently no driver supports that.
	 *    Note that drivers must wait for preceding updates to complete if a
	 *    synchronous update is requested, they are not allowed to fail the
	 *    commit in that case.
	 *
	 *  - -ENOMEM, if the driver failed to allocate memory. Specifically
	 *    this can happen when trying to pin framebuffers, which must only
	 *    be done when committing the state.
	 *
	 *  - -ENOSPC, as a refinement of the more generic -ENOMEM to indicate
	 *    that the driver has run out of vram, iommu space or similar GPU
	 *    address space needed for framebuffer.
	 *
	 *  - -EIO, if the hardware completely died.
	 *
	 *  - -EINTR, -EAGAIN or -ERESTARTSYS, if the IOCTL should be restarted.
	 *    This can either be due to a pending signal, or because the driver
	 *    needs to completely bail out to recover from an exceptional
	 *    situation like a GPU hang. From a userspace point of view all errors are
	 *    treated equally.
	 *
	 * This list is exhaustive. Specifically this hook is not allowed to
	 * return -EINVAL (any invalid requests should be caught in
	 * @atomic_check) or -EDEADLK (this function must not acquire
	 * additional modeset locks).
	 */
	int (*atomic_commit)(struct drm_device *dev,
			     struct drm_atomic_state *state,
			     bool nonblock);

	/**
	 * @atomic_state_alloc:
	 *
	 * This optional hook can be used by drivers that want to subclass struct
	 * &drm_atomic_state to be able to track their own driver-private global
	 * state easily. If this hook is implemented, drivers must also
	 * implement @atomic_state_clear and @atomic_state_free.
	 *
	 * RETURNS:
	 *
	 * A new &drm_atomic_state on success or NULL on failure.
	 */
	struct drm_atomic_state *(*atomic_state_alloc)(struct drm_device *dev);

	/**
	 * @atomic_state_clear:
	 *
	 * This hook must clear any driver private state duplicated into the
	 * passed-in &drm_atomic_state. This hook is called when the caller
	 * encountered a &drm_modeset_lock deadlock and needs to drop all
	 * already acquired locks as part of the deadlock avoidance dance
	 * implemented in drm_modeset_lock_backoff().
	 *
	 * Any duplicated state must be invalidated since a concurrent atomic
	 * update might change it, and the drm atomic interfaces always apply
	 * updates as relative changes to the current state.
	 *
	 * Drivers that implement this must call drm_atomic_state_default_clear()
	 * to clear common state.
	 */
	void (*atomic_state_clear)(struct drm_atomic_state *state);

	/**
	 * @atomic_state_free:
	 *
	 * This hook needs driver private resources and the &drm_atomic_state
	 * itself. Note that the core first calls drm_atomic_state_clear() to
	 * avoid code duplicate between the clear and free hooks.
	 *
	 * Drivers that implement this must call drm_atomic_state_default_free()
	 * to release common resources.
	 */
	void (*atomic_state_free)(struct drm_atomic_state *state);
};

/**
 * struct drm_mode_config - Mode configuration control structure
 * @mutex: mutex protecting KMS related lists and structures
 * @connection_mutex: ww mutex protecting connector state and routing
 * @acquire_ctx: global implicit acquire context used by atomic drivers for
 * 	legacy IOCTLs
 * @idr_mutex: mutex for KMS ID allocation and management
 * @crtc_idr: main KMS ID tracking object
 * @fb_lock: mutex to protect fb state and lists
 * @num_fb: number of fbs available
 * @fb_list: list of framebuffers available
 * @num_connector: number of connectors on this device
 * @connector_list: list of connector objects
 * @num_encoder: number of encoders on this device
 * @encoder_list: list of encoder objects
 * @num_overlay_plane: number of overlay planes on this device
 * @num_total_plane: number of universal (i.e. with primary/curso) planes on this device
 * @plane_list: list of plane objects
 * @num_crtc: number of CRTCs on this device
 * @crtc_list: list of CRTC objects
 * @property_list: list of property objects
 * @min_width: minimum pixel width on this device
 * @min_height: minimum pixel height on this device
 * @max_width: maximum pixel width on this device
 * @max_height: maximum pixel height on this device
 * @funcs: core driver provided mode setting functions
 * @fb_base: base address of the framebuffer
 * @poll_enabled: track polling support for this device
 * @poll_running: track polling status for this device
 * @output_poll_work: delayed work for polling in process context
 * @property_blob_list: list of all the blob property objects
 * @blob_lock: mutex for blob property allocation and management
 * @*_property: core property tracking
 * @degamma_lut_property: LUT used to convert the framebuffer's colors to linear
 *	gamma
 * @degamma_lut_size_property: size of the degamma LUT as supported by the
 *	driver (read-only)
 * @ctm_property: Matrix used to convert colors after the lookup in the
 *	degamma LUT
 * @gamma_lut_property: LUT used to convert the colors, after the CSC matrix, to
 *	the gamma space of the connected screen (read-only)
 * @gamma_lut_size_property: size of the gamma LUT as supported by the driver
 * @preferred_depth: preferred RBG pixel depth, used by fb helpers
 * @prefer_shadow: hint to userspace to prefer shadow-fb rendering
 * @async_page_flip: does this device support async flips on the primary plane?
 * @cursor_width: hint to userspace for max cursor width
 * @cursor_height: hint to userspace for max cursor height
 * @helper_private: mid-layer private data
 *
 * Core mode resource tracking structure.  All CRTC, encoders, and connectors
 * enumerated by the driver are added here, as are global properties.  Some
 * global restrictions are also here, e.g. dimension restrictions.
 */
struct drm_mode_config {
	struct mutex mutex; /* protects configuration (mode lists etc.) */
	struct drm_modeset_lock connection_mutex; /* protects connector->encoder and encoder->crtc links */
	struct drm_modeset_acquire_ctx *acquire_ctx; /* for legacy _lock_all() / _unlock_all() */
	struct mutex idr_mutex; /* for IDR management */
	struct idr crtc_idr; /* use this idr for all IDs, fb, crtc, connector, modes - just makes life easier */
	struct idr tile_idr; /* use this idr for all IDs, fb, crtc, connector, modes - just makes life easier */
	/* this is limited to one for now */

	struct mutex fb_lock; /* proctects global and per-file fb lists */
	int num_fb;
	struct list_head fb_list;

	int num_connector;
	struct ida connector_ida;
	struct list_head connector_list;
	int num_encoder;
	struct list_head encoder_list;

	/*
	 * Track # of overlay planes separately from # of total planes.  By
	 * default we only advertise overlay planes to userspace; if userspace
	 * sets the "universal plane" capability bit, we'll go ahead and
	 * expose all planes.
	 */
	int num_overlay_plane;
	int num_total_plane;
	struct list_head plane_list;

	int num_crtc;
	struct list_head crtc_list;

	struct list_head property_list;

	int min_width, min_height;
	int max_width, max_height;
	const struct drm_mode_config_funcs *funcs;
	resource_size_t fb_base;

	/* output poll support */
	bool poll_enabled;
	bool poll_running;
	bool delayed_event;
	struct delayed_work output_poll_work;

	struct mutex blob_lock;

	/* pointers to standard properties */
	struct list_head property_blob_list;
	struct drm_property *edid_property;
	struct drm_property *dpms_property;
	struct drm_property *path_property;
	struct drm_property *tile_property;
	struct drm_property *plane_type_property;
	struct drm_property *rotation_property;
	struct drm_property *prop_src_x;
	struct drm_property *prop_src_y;
	struct drm_property *prop_src_w;
	struct drm_property *prop_src_h;
	struct drm_property *prop_crtc_x;
	struct drm_property *prop_crtc_y;
	struct drm_property *prop_crtc_w;
	struct drm_property *prop_crtc_h;
	struct drm_property *prop_fb_id;
	struct drm_property *prop_crtc_id;
	struct drm_property *prop_active;
	struct drm_property *prop_mode_id;
	struct drm_property *content_protection_property;
	struct drm_property *content_protection_ksv_property;

	/* DVI-I properties */
	struct drm_property *dvi_i_subconnector_property;
	struct drm_property *dvi_i_select_subconnector_property;

	/* TV properties */
	struct drm_property *tv_subconnector_property;
	struct drm_property *tv_select_subconnector_property;
	struct drm_property *tv_mode_property;
	struct drm_property *tv_left_margin_property;
	struct drm_property *tv_right_margin_property;
	struct drm_property *tv_top_margin_property;
	struct drm_property *tv_bottom_margin_property;
	struct drm_property *tv_brightness_property;
	struct drm_property *tv_contrast_property;
	struct drm_property *tv_flicker_reduction_property;
	struct drm_property *tv_overscan_property;
	struct drm_property *tv_saturation_property;
	struct drm_property *tv_hue_property;

	/* Optional properties */
	struct drm_property *scaling_mode_property;
	struct drm_property *aspect_ratio_property;
	struct drm_property *dirty_info_property;

	/* Optional color correction properties */
	struct drm_property *degamma_lut_property;
	struct drm_property *degamma_lut_size_property;
	struct drm_property *ctm_property;
	struct drm_property *gamma_lut_property;
	struct drm_property *gamma_lut_size_property;

	/* properties for virtual machine layout */
	struct drm_property *suggested_x_property;
	struct drm_property *suggested_y_property;

	/* dumb ioctl parameters */
	uint32_t preferred_depth, prefer_shadow;

	/* whether async page flip is supported or not */
	bool async_page_flip;

	/* whether the driver supports fb modifiers */
	bool allow_fb_modifiers;

	/* cursor size */
	uint32_t cursor_width, cursor_height;

	struct drm_mode_config_helper_funcs *helper_private;
};

#define obj_to_crtc(x) container_of(x, struct drm_crtc, base)

extern __printf(6, 7)
int drm_crtc_init_with_planes(struct drm_device *dev,
			      struct drm_crtc *crtc,
			      struct drm_plane *primary,
			      struct drm_plane *cursor,
			      const struct drm_crtc_funcs *funcs,
			      const char *name, ...);
extern void drm_crtc_cleanup(struct drm_crtc *crtc);

/**
 * drm_crtc_index - find the index of a registered CRTC
 * @crtc: CRTC to find index for
 *
 * Given a registered CRTC, return the index of that CRTC within a DRM
 * device's list of CRTCs.
 */
static inline unsigned int drm_crtc_index(struct drm_crtc *crtc)
{
	return crtc->index;
}

/**
 * drm_crtc_mask - find the mask of a registered CRTC
 * @crtc: CRTC to find mask for
 *
 * Given a registered CRTC, return the mask bit of that CRTC for an
 * encoder's possible_crtcs field.
 */
static inline uint32_t drm_crtc_mask(struct drm_crtc *crtc)
{
	return 1 << drm_crtc_index(crtc);
}

extern void drm_crtc_get_hv_timing(const struct drm_display_mode *mode,
				   int *hdisplay, int *vdisplay);
extern int drm_crtc_force_disable(struct drm_crtc *crtc);
extern int drm_crtc_force_disable_all(struct drm_device *dev);

extern void drm_mode_config_init(struct drm_device *dev);
extern void drm_mode_config_reset(struct drm_device *dev);
extern void drm_mode_config_cleanup(struct drm_device *dev);

extern int drm_mode_create_dirty_info_property(struct drm_device *dev);
extern int drm_mode_set_config_internal(struct drm_mode_set *set);

extern struct drm_tile_group *drm_mode_create_tile_group(struct drm_device *dev,
							 char topology[8]);
extern struct drm_tile_group *drm_mode_get_tile_group(struct drm_device *dev,
					       char topology[8]);
extern void drm_mode_put_tile_group(struct drm_device *dev,
				   struct drm_tile_group *tg);

/* Helpers */
static inline struct drm_crtc *drm_crtc_find(struct drm_device *dev,
	uint32_t id)
{
	struct drm_mode_object *mo;
	mo = drm_mode_object_find(dev, id, DRM_MODE_OBJECT_CRTC);
	return mo ? obj_to_crtc(mo) : NULL;
}

#define drm_for_each_crtc(crtc, dev) \
	list_for_each_entry(crtc, &(dev)->mode_config.crtc_list, head)

static inline void
assert_drm_connector_list_read_locked(struct drm_mode_config *mode_config)
{
	/*
	 * The connector hotadd/remove code currently grabs both locks when
	 * updating lists. Hence readers need only hold either of them to be
	 * safe and the check amounts to
	 *
	 * WARN_ON(not_holding(A) && not_holding(B)).
	 */
	WARN_ON(!mutex_is_locked(&mode_config->mutex) &&
		!drm_modeset_is_locked(&mode_config->connection_mutex));
}

/* drm_bridge.c */
extern int drm_bridge_add(struct drm_bridge *bridge);
extern void drm_bridge_remove(struct drm_bridge *bridge);
extern struct drm_bridge *of_drm_find_bridge(struct device_node *np);
extern int drm_bridge_attach(struct drm_device *dev, struct drm_bridge *bridge);

bool drm_bridge_mode_fixup(struct drm_bridge *bridge,
			const struct drm_display_mode *mode,
			struct drm_display_mode *adjusted_mode);
void drm_bridge_disable(struct drm_bridge *bridge);
void drm_bridge_post_disable(struct drm_bridge *bridge);
void drm_bridge_mode_set(struct drm_bridge *bridge,
			struct drm_display_mode *mode,
			struct drm_display_mode *adjusted_mode);
void drm_bridge_pre_enable(struct drm_bridge *bridge);
void drm_bridge_enable(struct drm_bridge *bridge);

#endif /* __DRM_CRTC_H__ */
