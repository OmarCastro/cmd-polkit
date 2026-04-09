# Examples

These are examples on how to use `cmd-polkit` to create your own polkit agent

## Setup

Build and install cmd-polkit as defined in the project readme:

```bash
$ meson setup build
$ meson compile -C build
$ meson install -C build
```

## How to run examples


Simply execute the example, it will run `cmd-polkit-agent`.

All examples run on bash but some examples executes python, so it is recommended
to have bash and python installed as to run the examples