#!/usr/bin/env bash

sudo tee -a /etc/NetworkManager/conf.d/dns-servers.conf <<DNS
[global-dns-domain-*]
servers=::1,1.1.1.1
DNS

sudo systemctl restart NetworkManager
