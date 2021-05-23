# Realtime Users For Asterisk.
## Features
The current version allows you to:
* add,
* delete,
* edit

users placed in mysql database. 
Also allows to show users by:
* number,
* callerid,
* status,
* single account.

Extra-options: 
* making configs for provisioning grandstream gxp-1xxx, gxp-2xxx. At this time only for single account.
* reboot remote phones via ip-address.
* create needing tables in mysql database.
* truncate tables in mysql database.

## Dependencies
* gcc
* make
* libmysqlclient
* libcurl4
* git

Install all dependencies:

  sudo apt install gcc make libmysqlclient-dev libcurl4-openssl-dev git
## Building
  git clone https://github.com/vansatchen/rufa.git rufa
  
  cd rufa
  
  make
  
  sudo make install
## Config
Config may be located at current directory as rufa.conf , at home directory as .rufarc, at /etc as rufa.conf
## Usage
* rufa --help - usage help
* rufa --createdb - create needing tables
* rufa --truncatedb - truncate tables
* rufa somecontext - 'somecontext' as default context in interactive mode
* rufa - interactive mode
