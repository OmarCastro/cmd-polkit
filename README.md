# cmd-polkit
<a href="https://github.com/OmarCastro/cmd-polkit" aria-label="go to Github repository" title="go to Github repository">
<picture>
    <img src="https://omarcastro.github.io/cmd-polkit/reports/license-badge-a11y.svg">
</picture>
</a><a href="https://omarcastro.github.io/cmd-polkit/reports/testlog.txt" aria-label="Show test results">
<picture>
    <img src="https://omarcastro.github.io/cmd-polkit/reports/test-results-badge-a11y.svg">
</picture>
</a><a href="https://omarcastro.github.io/cmd-polkit/reports/coveragereport" aria-label="Show test code coverage information">
<picture>
    <img src="https://omarcastro.github.io/cmd-polkit/reports/coverage-badge-a11y.svg">
</picture>
</a>

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
