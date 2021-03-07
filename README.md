# cmd-polkit

A tool that allows to easily customize the UI used to authenticate on polkit

### Dependencies

| Dependency   | Version |
|--------------|---------|
| glib 	       | 2.0     |
| json-glib    | 1.0     |
| polkit-agent | 2.0     |
| gtk+ 	       | 3.0     |

# Instalation

 It requires [meson](https://mesonbuild.com/index.html) build system and the dependencies installed
 
  
```bash
$ meson setup build
$ meson compile -C build
$ meson install -C build
```
