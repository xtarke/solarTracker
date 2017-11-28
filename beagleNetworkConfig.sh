INTERNET=enp7s0
LOCAL=$1   #BealgleBone


sysctl net.ipv4.ip_forward=1
iptables -t nat -A POSTROUTING -o $INTERNET -j MASQUERADE
iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i $LOCAL -o $INTERNET -j ACCEPT

#In devices:
# # ip route add default via <server-ip> dev  <local iface >   # same address as in the beginning

ip link set $LOCAL up
ip addr add 192.168.7.1/30 broadcast 192.168.2.255 dev $LOCAL
#ip route add default via 192.168.7.1 dev usb0
