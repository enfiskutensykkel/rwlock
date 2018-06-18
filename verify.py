#!/usr/bin/env python

import sys
import re
import numpy as np


writer_acquire = re.compile(r'^\[(?P<thread>\d+)\] taking write lock')
writer_release = re.compile(r'^\[(?P<thread>\d+)\] releasing write lock')

reader_wait_expr = re.compile(r'^\[(?P<thread>\d+)\] taking read lock \(value=\d+, readers=\d+, waiting writers=\d+, waited=(?P<time>\d+)\)$')


if len(sys.argv) != 2:
    print >> sys.stderr, "Usage: {} <filename>".format(sys.argv[0])
    sys.exit(1)


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

        elif n:
            m = writer_acquire.match(lines[i - 1])

            if not m:
                raise "Write lock acquiring not immediately followed by release"

            thread_take = int(m.group('thread'))
            thread_release = int(n.group('thread'))

            if thread_take != thread_release:
                raise "Write lock not released by same thread"

        elif e:
            thread = int(e.group('thread'))
            elapsed = int(e.group('time'))

            all_read_times.append(elapsed)

reads = np.array(all_read_times)

print ".90:", np.percentile(reads, .90)
print ".95:", np.percentile(reads, .95)
print ".97:", np.percentile(reads, .97)
print ".99:", np.percentile(reads, .99)
print "max:", np.max(reads)
