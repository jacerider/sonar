# Sonar - SASS compiler for Drupal

    ███████╗ ██████╗ ███╗   ██╗ █████╗ ██████╗
    ██╔════╝██╔═══██╗████╗  ██║██╔══██╗██╔══██╗
    ███████╗██║   ██║██╔██╗ ██║███████║██████╔╝
    ╚════██║██║   ██║██║╚██╗██║██╔══██║██╔══██╗
    ███████║╚██████╔╝██║ ╚████║██║  ██║██║  ██║
    ╚══════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═╝

## How to install

Just enable the module. Any files added through drupal_add_css or theme .info
files will be compiled into a single SASS file.

## How to extend

Implementations that wish to provide a new SCSS->CSS adapter should register
it using CTools' plugin system.
