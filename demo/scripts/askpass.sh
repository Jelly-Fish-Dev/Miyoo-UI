#!/bin/sh
# SSH_ASKPASS helper used by the Makefile: answers ssh's password prompt with
# $DEMO_SSH_PASS (set in local.mk). Prefer key auth (ssh-copy-id) when possible.
printf '%s\n' "$DEMO_SSH_PASS"
