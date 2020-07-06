# SetMonitorBrightness

Quick and dirty command line utility to set the brightness of all monitors
that support DDC/CI.

There are a few alternatives out there, but they were buggy or didn't meet
my immediate needs. e.g. They required you to manually adjust a slider for
each monitor, and when running continuously seemed to forget monitors when
they, or the machine, went to sleep.

Usage:

SetMonitorBrightness.exe brightness

Per-monitor brightness isn't supported; it tries to set all monitors to the
same level. My usage is to create shortcuts and pin them to the Start menu
for "Daytime Brightness" and "Nighttime Brightness".
