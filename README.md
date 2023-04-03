# 433-ip-phone

## Run the node app locally
```
$(host) make
$(bbg) cd /mnt/remote/myApps/ip_phone_node_copy
$(bbg) node server.js
```
App url: 192.168.7.2:8080


Intial account url is hard coded modify this using getaddr.. fucntion call 
pjsua module starts up similar to udp module, currently fucntion dependent on cli modify this to be dependedent on the fucntion calls which are thread safe
