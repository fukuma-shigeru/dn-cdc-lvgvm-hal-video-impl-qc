dn-cdc-lvgvm-hal-video-impl-qc

# How to build


```bash
$ git clone https://github.com/bevs3-cdc/dn-cdc-lvgvm-hal-video-impl-qc.git

$ mkdir -p dn-cdc-lvgvm-hal-video-impl-qc/build

$ cd dn-cdc-lvgvm-hal-video-impl-qcc/build

$ cmake ..

$ make all

$ make package

```

```
$ dpkg -c dn-cdc-lvgvm-hal-video-impl-qc_1.0.0-r0_arm64.deb 
drwxrwx--- root/root         0 2021-11-18 11:36 ./lib/
drwxrwx--- root/root         0 2021-11-18 11:36 ./lib/systemd/
drwxrwx--- root/root         0 2021-11-18 11:36 ./lib/systemd/system/
-rwxrwx--- root/root       266 2021-11-18 09:14 ./lib/systemd/system/videohal.service
drwxrwx--- root/root         0 2021-11-18 11:36 ./opt/
drwxrwx--- root/root         0 2021-11-18 11:36 ./opt/dc-ivi-pf/
drwxrwx--- root/root         0 2021-11-18 11:36 ./opt/dc-ivi-pf/bin/
-rwxrwx--- root/root    193712 2021-11-18 11:36 ./opt/dc-ivi-pf/bin/video_hal_svc
```

