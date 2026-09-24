#!/usr/bin/env python3
"""Fail if a workflow's push or pull_request trigger can never match.

A branch filter naming a branch that does not exist leaves the workflow
configured but never scheduled, which looks identical to a workflow that
runs and passes: no red check appears, because no check appears at all.

Only filters made entirely of literal names are judged. A filter holding
any glob is left alone, since whether it can match depends on branches
that may not exist yet.

  usage: check_workflow_triggers.py [workflow-dir]
"""
import glob
import os
import subprocess
import sys

import yaml

GLOB_CHARS = set('*?[]!+@')


def existing_branches():
    """Branch names on the remote, falling back to local refs."""
    for cmd in (['git', 'ls-remote', '--heads', 'origin'],
                ['git', 'for-each-ref', '--format=%(refname)', 'refs/heads']):
        out = subprocess.run(cmd, capture_output=True, text=True)
        if out.returncode == 0 and out.stdout.strip():
            names = set()
            for line in out.stdout.split('\n'):
                ref = line.split('refs/heads/')[-1].strip()
                if ref:
                    names.add(ref)
            if names:
                return names
    return set()


def default_branch():
    env = os.environ.get('DEFAULT_BRANCH')
    if env:
        return env.strip()
    out = subprocess.run(['git', 'symbolic-ref', '--short', 'refs/remotes/origin/HEAD'],
                         capture_output=True, text=True)
    if out.returncode == 0 and out.stdout.strip():
        return out.stdout.strip().split('/')[-1]
    return None


def triggers(doc):
    """The `on:` mapping. PyYAML reads an unquoted `on` key as True."""
    for key in (True, 'on', 'On', 'ON'):
        if isinstance(doc, dict) and key in doc:
            return doc[key]
    return None


def main(argv):
    where = argv[1] if len(argv) > 1 else '.github/workflows'
    branches = existing_branches()
    default = default_branch()
    if not branches:
        print('could not determine the repository branches; nothing checked')
        return 0
    print('branches on the remote: %d, default: %s'
          % (len(branches), default or 'unknown'))

    problems = []
    for path in sorted(glob.glob(os.path.join(where, '*.yml'))
                       + glob.glob(os.path.join(where, '*.yaml'))):
        try:
            doc = yaml.safe_load(open(path))
        except yaml.YAMLError as exc:
            problems.append('%s: cannot parse: %s' % (path, exc))
            continue
        on = triggers(doc)
        if not isinstance(on, dict):
            continue
        for event in ('push', 'pull_request', 'pull_request_target'):
            spec = on.get(event)
            if not isinstance(spec, dict):
                continue
            names = spec.get('branches')
            if not names:
                continue
            if any(GLOB_CHARS & set(n) for n in names):
                continue
            live = [n for n in names if n in branches]
            if not live:
                problems.append(
                    '%s: %s.branches names only %s, and no such branch exists.\n'
                    '    This workflow can never be scheduled.%s'
                    % (path, event, ', '.join(repr(n) for n in names),
                       ('\n    The default branch is %r.' % default) if default else ''))
            elif default and default not in names:
                print('note: %s: %s.branches does not include the default '
                      'branch %r' % (path, event, default))

    if problems:
        print('\nWorkflow trigger check failed:\n')
        for p in problems:
            print('  ' + p)
        return 1
    print('every push and pull_request trigger can match an existing branch')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
