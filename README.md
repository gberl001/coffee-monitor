# Environment Setup
## Requirements
* Must have Python installed
* Must have PlatformIO CLI (PlatformIO Core) installed and updated to the latest
  * If not updated to the latest, your project may not be structured properly

## Project Structure Setup
From command line, simply run

    pio init --board nanoatmega328 --ide clion

### Notes:
You may update the `--ide` option if you prefer to use a different IDE but I have not tested this so
there is no guarantee that it will be structured properly.
# Building & Uploading
## Building
From the command line, simply run

    pio run
    
The port your board is connected to should be detected by default but there is a configuration that 
can be set in the platform.ini file to specify a port. There may also be an argument that can be
added to the command so as not to require altering the project file.
## Uploading
From the command line, simply run

    pio run --target upload
