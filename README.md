# IoT-project2023

## General Information

This is the mini-projet number 1 for the IoT-course in 2023. Project was done as a solo project by Roope Karjalainen.

Project is as minimalistic as it can but it is demonstrating how we can get data from IoT-testbed to the Cloud. In this project we are using AWS cloud as our cloud service and CoAP as our communication protocol between the nodes.

## Steps

### Step 1

First of all I needed to decide what kind of sensor I am going to use and how we are going to utilize it. In my bigger picture I visioned a  system that could measure light and humidity that could be used for example to monitor well-being of houseplants.

Doing the project solo and the difficulty of the project adjusted to my work and school schedule I decided that I am only going to read the light sensor data using the iotlab-m3 board.

After deciding I had to create the functionality so that I can read the sensor data. I did this by going through the demos and utilizing the materials to create components to main.c application and adding the needed components to the Makefile.

### Step 2

Next I needed to get the message from one node to another. For this task I used tutorials from RIOT (gnrc networking and coap). I added the needed components to main.c and Makefile plus I added the gcoapcli.c to the project. After that I made sure that the messages between two nodes are going through with IoT-LAB experiment.

### Step 3

Next I needed connectivity through IPV6 so that I can communicate globally to the cloud server. To help with this I followed the gnrc_border_router tutorial by RIOT. To ensure this part was working I tested it with IoT-Lab experiment and pinging from outside to the IoT-Lab node.

### Step 4

I needed a VM in the cloud service that has IPV6 connectivity so that I can send and receive data from the cloud through the border router node to the sensing node. For this I used AWS like it was recommended in the discord server and set a security group inbound rule so that IPV6 is enabled. After that I set a public IPV6 address for the VM. After this was done I tested the connectivity from the VM to the sensing node by pinging the nodes global IPv6 address.

### Step 5

After that I needed a CoAP client on the VM so that I can get the sensor data from the node through CoAP. For this I used aiocoap (https://aiocoap.readthedocs.io/en/latest/). With aiocoap my VM is working as a CoAP client and the sensing node is working as a CoAP server. I tested the connectivity from sensing node to VM by asking the sensor data from VM.

### Improvements

This project is bare minimum and the changes I would've loved to make like database for the results (and adding some analyzable data like timestamps) and optimizing the sensor layer are future improvements.

## Instructions for testing the project

### First of all

1. You need to be able to connect to the FIT IoT-lab servers. We are using grenoble (other sites were for some reason getting errors) as our example location. So connect to the servers from the command line:

   ```ssh <USERNAME>@grenoble.iot-lab.info```

2. Clone RIOT repository (https://github.com/RIOT-OS/RIOT) to your home directory.

   ```git clone https://github.com/RIOT-OS/RIOT.git -b 2022.01```

4. Create a project file that includes the files main.c, gcoapcli.c and Makefile from this git repository.

   ```mkdir project```

   And add the files to that directory.

5. Create VM to AWS that has IPv6 connectivity: create instance (for example EC2)

   create security group with inbound rule that enables IPv6 connectivity

   set the security group for your EC2 instance and set public IPv6 address for your EC2 instance

   tutorials for this: https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/using-instance-addressing.html

6. Start and connect to your VM and install aiocoap with the instructions https://aiocoap.readthedocs.io/en/latest/installation.html and add aiocoap-project file to the aiocoap directory. Then set permissions for the file with

   ```chmod 775 aiocoap-project```

### Actual testing of the project

1. Connect to the lille site with two terminals

   ```ssh <USERNAME>@lille.iot-lab.info```

3. Enter ```source /opt/riot.source``` to both of them

4. Go to your project directory

   ```cd project```

5. Create experiment with two nodes:

   ```iotlab-experiment submit -n "iot-project" -d 60 -l 2,archi=m3:at86rf231+site=lille```

6. Wait until the experiment is in "Running" state

   ```iotlab-experiment wait --timeout 30 --cancel-on-timeout```

7. Check IDs of the nodes:

   ```iotlab-experiment --jmespath="items[*].network_address | sort(@)" get --nodes```

8. Make your board with specific default_channel and default_pan_id. We are using DEFAULT_CHANNEL=19 DEFAULT_PAN_ID=0xdfca

   ```make DEFAULT_CHANNEL=19 DEFAULT_PAN_ID=0xdfca```

9. Flash created bin files to the nodes:

   ```iotlab-node --flash ./bin/iotlab-m3/iotproject.bin```

10. go to the nodes with both terminals

    ```nc m3-<NODE_ID> 20000```

11. test the connectivity between the nodes by checking the ip address of the node number one and pinging it from node number two:

    ```> ifconfig```

    ```> ping <IP_ADDRESS>```

12. After this disconnect from one node using Ctrl+C and make the border router(place node id for the border router to $BR_ID:

    ```make -C ../RIOT/examples/gnrc_border_router/ ETHOS_BAUDRATE=500000 BOARD=iotlab-m3 DEFAULT_CHANNEL=19 DEFAULT_PAN_ID=0xdfca IOTLAB_NODE=m3-$BR_ID.lille.iot-lab.info```

13. Then flash it to the correct node with (where <id> is the node id that is not used for the border router):

    ```iotlab-node --flash /senslab/users/iot2023oulu101/RIOT/examples/gnrc_border_router/bin/iotlab-m3/gnrc_border_router.elf -e lille,m3,<id>```

14. Then check the unavailable taps with:

    ```ip addr show | grep tap```

15. and unavailable routes with:

    ```ip -6 route```

16. From these you have to choose available tap and route for next step. So for example if taps tap0 and tap1 are unavailable you can choose tap2. For route you can choose first one that is available. Grenoble routes start from 2001:660:5307:3100::/64 So you check the first that is not unavailable and you choose that one.

17. Then you deploy the border router connection with (<br_id> is the id of your border router node and you replace <available_tap> and <available_route> with your chosen tap and route:

    ```sudo ethos_uhcpd.py <br_id> <available_tap> <available_route>```

18. You can that the border router is running with ```nc m3-<br_id> 20000``` (this is not necessary but if there are some problems)

19. Then you start checking from the other node where the console is open that you get the global address with ```ifconfig``` (this usually takes some time to see. for about 10s-60s)

20. When we have the global IPv6 address we can move to the VM on AWS.

21. Start the VM and connect to it.

22. Ping the nodes from AWS

    ```ping <global_IPv6_address>```

23. Go to aiocoap folder on AWS VM

    ```cd aiocoap```

24. Launch the aiocoap-project program with the global ipv6 address:

    ```./aiocoap-project coap://[public_ipv6_address]/light```

25. You start getting the actual sensor data from the sensing node through the border router to the AWS VM.

## Video demonstration

https://youtu.be/A_b0kWcJboI
