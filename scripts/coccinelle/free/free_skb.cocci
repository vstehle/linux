// SPDX-License-Identifier: GPL-2.0-only
/// Rules for a networking driver transmit hook (ndo_start_xmit() method) to
/// free up the socket buffer:
/// - If the DMA mapping fails you must call dev_kfree_skb() to free the socket
///   buffer and return NETDEV_TX_OK.
/// - If you return NETDEV_TX_BUSY, you must not attempt to free it up.
///
// Copyright: (C) 2020 Vincent Stehlé.
// Options: --no-includes --include-headers

virtual patch
virtual org
virtual report
virtual context

@ndo@
identifier ops, start_xmit_f;
@@

struct net_device_ops ops = {
  ...,
  .ndo_start_xmit = start_xmit_f,
  ...
};

// TODO! Check free + return ok after a dma mapping error

@xmit_p depends on ndo && patch@
identifier ndo.start_xmit_f, skb;
@@

start_xmit_f(struct sk_buff *skb, ...)
{
  <...
(
- kfree_skb(skb);
|
- dev_kfree_skb(skb);
|
- dev_kfree_skb_irq(skb);
|
- dev_kfree_skb_any(skb);
|
- consume_skb(skb);
)
  ...
  return NETDEV_TX_BUSY;
  ...>
}

@xmit_r depends on ndo && (context || report || org)@
identifier ndo.start_xmit_f, skb;
position p;
@@

* start_xmit_f(struct sk_buff *skb, ...)
{
  <+...
(
* kfree_skb@p(skb);
|
* dev_kfree_skb@p(skb);
|
* dev_kfree_skb_irq@p(skb);
|
* dev_kfree_skb_any@p(skb);
|
* consume_skb@p(skb);
)
  ...
* return NETDEV_TX_BUSY;
  ...+>
}

@script:python depends on ndo && xmit_r && org@
p << xmit_r.p;
@@

cocci.print_main("freeing skb before returning NETDEV_TX_BUSY", p)

@script:python depends on ndo && xmit_r && report@
start_xmit_f << ndo.start_xmit_f;
p << xmit_r.p;
@@

msg = "WARNING: %s() freeing skb before returning NETDEV_TX_BUSY." % (start_xmit_f)
coccilib.report.print_report(p[0], msg)
