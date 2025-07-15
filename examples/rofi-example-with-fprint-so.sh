#!/usr/bin/env bash

<< ////

This example expects you to have fprint setup. Make sure you follow the instructions for your distro.

Setup
======

Generally you should have it setup with "fprintd-enroll" command, pointed to a finger choosen
as an argument of the executable

I leave are some links for some distros I found while learning how to configure it on my machine

Linux Mint, last seen on 2025-06-26: https://forums.linuxmint.com/viewtopic.php?t=408129
Arch Linux, last seen on 2025-06-26: https://wiki.archlinux.org/title/Fprint

After setup with fprint you will need to setup PAM on polkit. 
The polkit PAM configuration is in "/etc/pam.d/polkit-1" file

Depending on each distro you can configure in 1 or 2 ways:

1. With "pam_fprintd.so": 
  Prompts for a password first; if password fails, it will proceed to fingerprint authentication. 

2. With "pam_fprintd_grosshack.so": 
  Prompts for a password and fingerprint at the same time, This may be needed for some graphical programs which
  do not allow blank password input, such as Gnome's built-in polkit agent.


This example was made for "pam_fprintd.so" config, so you can apply the configuration by adding the following 2 lines
at the top of /etc/pam.d/polkit-1:

"""
auth		sufficient  	pam_unix.so try_first_pass likeauth nullok
auth		sufficient  	pam_fprintd.so
...
"""

If /etc/pam.d/polkit-1 does not exist, you copy from /usr/lib/pam.d/polkit-1 first before adding the previous 2 lines.

Once again, if it fails or if there is any issue regarding polkit configuration, check the instructions on how to configure
for your distro.


How to use this example
=======================

1. run this executable
2. run any command that requires polkit autentication (e.g. "pkexec echo 123")
3. When rofi opens press Enter wihout writing the password
4. Should appear a message to put a specific finger on the fingerprint scanner, put the indicated finger on the scanner
4a. If it fails, a new line showing the error should appear, and the a new info line should appear to put a finger to
    the fingerprint again, put the indicated finger on the fingerprint scanner
5. Fingerprint scanned successfully and the authentication is successful!



Have fun
Omar Castro

////

cd "$(dirname "${BASH_SOURCE[0]}")"
../build/cmd-polkit-agent -s -c "python scripts/rofi-example-with-fprintd-so.py"
