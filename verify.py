#!/usr/bin/env python

import sys
import re


writer_acquire = re.compile(r'^\[(?P<thread>\d+)\]\s+taking write lock')
writer_release = re.compile(r'^\[(?P<thread>\d+)\]\s+releasing write lock')


if len(sys.argv) != 2:
    print >> sys.stderr, "Usage: {} <filename>".format(sys.argv[0])
    sys.exit(1)


with open(sys.argv[1]) as fp:
    lines = fp.readlines()
    for i, line in enumerate(lines):
        m = writer_acquire.match(line)
        n = writer_release.match(line)

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

