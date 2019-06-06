## Install SSL

```
sudo apt-get -y install openssl
```

## Create private RSA key:

```
sudo mkdir /etc/certs
cd /etc/certs/
sudo openssl genrsa -aes256 -out server.key 2048
sudo cp server.key server.key.org
sudo openssl rsa -in server.key.org -out server.key
sudo rm server.key.org
```

## Make cert valid for 10 years:

```
openssl req -new -sha256 -days 3650 -key server.key -out server.csr
```
- Fill all fields.

## Sign the certificate:

```
sudo openssl x509 -req -days 3650 -sha256 -in server.csr -signkey server.key -out server.crt
```

## Change permissions:

```
sudo chmod uga+r  /etc/certs/server.crt
sudo chmod uga+r  /etc/certs/server.key
```

## Change grafana config file: /etc/grafana/grafana.ini

```
protocol = https
http_port = 3000
cert_file = /etc/certs/server.crt
cert_key = /etc/certs/server.key
```

Based on [link](http://blog.aeciopires.com/configurando-o-grafana-para-funcionar-sobre-https/)

