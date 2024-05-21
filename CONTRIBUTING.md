# Contributing to "cmd-polkit"

## Code of conduct

 This project has adopted the Contributor Covenant as its Code of Conduct, and we expect project
participants to adhere to it. Please read the full text so that you can understand what actions
will and will not be tolerated.

## Open Development

 All work on "cmd-polkit" happens directly on GitHub. Both team members and contributors send pull
requests which go through the same review process

## Versioning Policy

 "cmd-polkit" follows semantic versioning. We release patch versions for critical bugfixes, minor
versions for new features or non-essential changes, and major versions for any breaking changes.
 When we make breaking changes, we also introduce deprecation warnings in a minor version so that
our users learn about the upcoming changes and migrate their code in advance.

Every significant change is documented in the CHANGELOG.md file.

## Branch Organization

 Submit all changes directly to the main branch. We don’t use separate branches for development or
for upcoming releases. We do our best to keep main in good shape, with all tests passing.

 Code that lands in main must be compatible with the latest stable release. It may contain additional
features, but no breaking changes. We should be able to release a new minor version from the tip of
main at any time

## Issues

 We are using GitHub Issues for our bugs. We keep a close eye on this and try to make it
clear when we have an internal fix in progress. Before filing a new task, try to make sure your
problem does not already exist.


## Contribution Prerequisites

- You have a C compiler and meson installed at latest stable.
- you have the following dependencies

  | Dependency   | Version |
  |--------------|---------|
  | glib 	       | 2.0     |
  | json-glib    | 1.0     |
  | polkit-agent | 2.0     |
  | gtk+ 	       | 3.0     |

- You are familiar with Git.
- For testing, you have valgrind installed
- For documentation: you have NodeJs installed (recommended to use the latest LTS version)

## Development Workflow

After cloning the project's code repository, you can run several commands that are the shell scripts in `dev-tools` folder.

- `dev-tools/continuous-build.sh` builds the project and run tests automatically for time there is a code file change.
- `dev-tools/github-action-build.sh` builds the project, tests, publishes test reports and builds documentation, used for github pages workflow.
- `dev-tools/hot-reload.sh` builds the project and runs cmd-polkit using `examples/scripts/rofi-example.py` as the command. Each time a code file changes, rebuilds the project and restarts cmd-polkit with the updated code. Keep in mind that only one polkit agent is allowed at the same time.
- `dev-tools/run` simply runs NodeJs scripts for documentation purposes, run `dev-tools/run help` to list the scripts