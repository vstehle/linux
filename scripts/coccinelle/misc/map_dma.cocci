// SPDX-License-Identifier: GPL-2.0-only
/// Check dma_map_{single,page}() return value with dma_mapping_error()
///
// Copyright: (C) 2020 Vincent Stehlé.
// Options: --no-includes --include-headers

virtual context
virtual patch
virtual org
virtual report

@dma_map_p depends on patch@
expression dma_addr, dev;
@@

(
dma_addr = dma_map_single(dev, ...);
|
dma_addr = dma_map_page(dev, ...);
)
...
- (!dma_addr)
+ dma_mapping_error(dev, dma_addr)

@dma_map_r depends on !patch@
expression dma_addr, dev;
position p1, p2;
@@

(
dma_addr = dma_map_single@p1(dev, ...);
|
dma_addr = dma_map_page@p1(dev, ...);
)
...
* (!dma_addr@p2)

@script:python depends on report@
p1 << dma_map_r.p1;
p2 << dma_map_r.p2;
@@

msg="WARNING: address mapped with dma_map_{single,page}() on line %s should be checked with dma_mapping_error()" % (p1[0].line)
coccilib.report.print_report(p2[0], msg)

@script:python depends on org@
p2 << dma_map_r.p2;
@@

cocci.print_main("Use dma_mapping_error()",p2)
