#!/usr/bin/env python

import sys
import re
import numpy as np


writer_acquire = re.compile(r'^\[(?P<thread>\d+)\] taking write lock')
writer_release = re.compile(r'^\[(?P<thread>\d+)\] releasing write lock')

reader_wait_expr = re.compile(r'^\[(?P<thread>\d+)\] taking read lock \(waited=(?P<time>\d+)\)$')


if len(sys.argv) != 2:
    print >> sys.stderr, "Usage: {} <filename>".format(sys.argv[0])
    sys.exit(1)

writers = set()
readers = set()

all_read_times = []
with open(sys.argv[1]) as fp:
    lines = fp.readlines()
    for i, line in enumerate(lines):
        m = writer_acquire.match(line)
        n = writer_release.match(line)

        e = reader_wait_expr.match(line)

        if m:
            n = writer_release.match(lines[i + 1])

            if not n:
                raise "Write lock acquiring not immediately followed by release"

            thread_take = int(m.group('thread'))
            thread_release = int(n.group('thread'))

            if thread_take != thread_release:
                raise "Write lock not released by same thread"

            writers.add(thread_take)
            writers.add(thread_release)

        elif n:
            m = writer_acquire.match(lines[i - 1])

            if not m:
                raise "Write lock acquiring not immediately followed by release"

            thread_take = int(m.group('thread'))
            thread_release = int(n.group('thread'))

            if thread_take != thread_release:
                raise "Write lock not released by same thread"

            writers.add(thread_take)
            writers.add(thread_release)

        elif e:
            thread = int(e.group('thread'))
            elapsed = int(e.group('time'))

            readers.add(thread)

            all_read_times.append(elapsed)

reads = np.array(all_read_times)


print "Reader threads:", len(readers)
print "Writer threads:", len(writers)

print "min :", "%14.3f" % np.min(reads)
for percentile in [.05, .10, .25, .50, .75, .90, .95, .97, .99]:
    print "%4.2f:" % percentile, "%14.3f" % np.percentile(reads, percentile)
print "max :", "%14.3f" % np.max(reads)
