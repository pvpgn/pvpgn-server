Player vs Player Gaming Network - PRO on Docker
=====

To quickly deploy a PVPGN server (especially on Linux), using Docker is one of the best choices. You can gain some benefits using this:
- Small image size: approx. 234MB uncompressed, 57MB compressed image size (with single backend and data storage support, eg: `bnetd` and `mysql`)
- Quick to deploy, easy to scale out and update
- Simple to use and configurate

## Prequisite:

- Docker (Check [this link](https://docs.docker.com/engine/install/) for installation manual). You can use Podman instead of Docker, but in this manual we will use Docker CLI command so you may need Podman with Docker CLI support
- (optional) `docker-compose` for better configuration, deployment and logging managing

## How to build:

There are some supported build arguments:

| Build Argument | Original CMAKE Option | Default Value |
|----------------|-----------------------|---------------|
| with_bnetd     | WITH_BNETD            | true          |
| with_d2cs      | WITH_D2CS             | false         |
| with_d2dbs     | WITH_D2DBS            | false         |
| with_lua       | WITH_LUA              | false         |
| with_mysql     | WITH_MYSQL            | true          |
| with_sqlite3   | WITH_SQLITE3          | false         |
| with_pgsql     | WITH_PGSQL            | false         |
| with_odbc      | WITH_ODBC             | false         |

And some extra build args which is not related to CMAKE:

| Build Argument | Description                       | Default Value |
|----------------|-----------------------------------|---------------|
| git_repo       | The repository you want to clone  | this repo url |
| git_branch     | The branch you want to build from | develop       |

Adding above build arguments to enable/disable features (Image tagging will helps you distinguish image capabilities/features)

There are some enviroment variables you might want to override:

| Env Var     | Description                                             | Default Value |
|-------------|---------------------------------------------------------|---------------|
| SERVER_TYPE | Server type, choose between `d2cs`, `d2dbs` and `bnetd` | `bnetd`       |

Choose the appropriate `SERVER_TYPE` base on your image build

Below are some sample build commands:

```bash
# This command will use all default build arguments and build PVPGN docker image with only bnetd and MySQL support using master branch
docker build . -t pvpgn-server:bnetd-mysql

# This command will build a PVPGN with only d2cs and sqlite3 support using develop branch
docker build . \
  --build-arg with_d2cs=true \     # enable d2cs support (disable by default)
  --build-arg with_bnetd=false \   # enable bnetd support (enable by default)
  --build-arg with_mysql=false \   # enable MySQL support (enable by default)
  --build-arg with_sqlite3=true \  # enable sqlite3 support (disable by default)
  --build-arg git_branch=develop \ # switch to develop branch (master branch by default)
  -t pvpgn-server:d2cs-sqlite      # tag the image 
```

## How to build, run and manage:

Below are a sample build and run session.

### Common preparation:

Before doing anything, you will need to prepare the images and configuration files. The example below will helps you to build PVPGN servers with `mysql` support.

1. Prepare images:

```bash
docker build . -t pvpgn-server:bnetd-mysql
docker build . --build-arg with_d2cs=true --build-arg with_bnetd=false -t pvpgn-server:d2cs-mysql
docker build . --build-arg with_d2dbs=true --build-arg with_bnetd=false -t pvpgn-server:d2dbs-mysql
```

2. Prepare configuration files and assets (skip this step if you've already had them):

```bash
# create configuration and asset locations
mkdir -p /opt/pvpgn/etc /opt/pvpgn/var
# copy configuration files and base assets from an image, all base configuration files in any image are the same
# and btw yes, this command is not a mistake, please read ENTRYPOINT and CMD in Dockerfile specification for further explain
docker run -v /opt/pvpgn/etc:/tmp/conf --rm --entrypoint cp pvpgn-server:bnetd-mysql -r /usr/local/etc/pvpgn/* /tmp/conf
docker run -v /opt/pvpgn/var:/tmp/assets --rm --entrypoint cp pvpgn-server:bnetd-mysql -r /usr/local/etc/pvpgn/* /tmp/assets
```
Modify the copied configuration files to fit your needs. There are examples to configurating the server to use database in container, please check below. To configurate more, check [this link](https://pvpgn.pro/pvpgn_installation.html) to get more information of each components of the configuration files.


### Using docker only:

To run the images, you will need to mount existed configuration files and assets which is located at `/usr/local/etc/pvpgn` and `/usr/local/var/pvpgn` inside the container. If you don't have those, you can obtain them by following the above instruction. The example bellow shows how to turn up some PVPGN containers by some images prepared above:

```bash
mkdir -p /opt/var/lib/mysql # create MySQL data directory
# start MySQL to store data
docker run -d \              # run detached
  --name mysql \             # set a name for MySQL
  --restart unless-stopped \ # auto restart the container if it crash or server restart, unless you manually stop it
  # set some required MySQL environment variables
  -e MYSQL_ROOT_PASSWORD='example' \ # password of user root
  -e MYSQL_USER='bnetd' \            # bnetd MySQL user
  -e MYSQL_PASSWORD='example' \      # bnetd MySQL password
  -e MYSQL_DATABASE='bnetd' \        # the only database which bnetd user can access
  -v /opt/var/lib/mysql:/var/lib/mysql \ # mount database storage to the host
  mysql:8.0 --default-authentication-plugin=mysql_native_password # use native password instead of default "caching_sha2_password" authentication plugin, which is not supported by php and a lot legacy softwares

# run pvpgn with bnetd and MySQL support
docker run -d \
  --name pvpgn-bnetd \
  --restart unless-stopped \
  -p 6112:6112 -p 6112:6112/udp \          # forwarding default bnetd port with both tcp and udp protocol
  -v /opt/pvpgn/etc:/usr/local/etc/pvpgn \ # configuration mounting
  -v /opt/pvpgn/var:/usr/local/var/pvpgn \ # assets mounting
  pvpgn-server:bnetd-mysql                 # bnetd with MySQL support image

# run pvpgn with d2cs and MySQL support
docker run -d \                   
  --name pvpgn-d2cs \            
  --restart unless-stopped \
  -e SERVER_TYPE=d2cs \           # override bnetd server type to d2cs
  -p 6113:6113 -p 6113:6113/udp \ # forwarding default d2cs port
  -v /opt/pvpgn/etc:/usr/local/etc/pvpgn \ 
  -v /opt/pvpgn/var:/usr/local/var/pvpgn \ 
  pvpgn-server:d2cs-mysql         # d2cs with MySQL support image

# run pvpgn with d2dbs and MySQL support
docker run -d \
  --name pvpgn-d2dbs \
  --restart unless-stopped \
  -e SERVER_TYPE=d2dbs \          # override bnetd server type to d2dbs
  -p 6114:6114 -p 6114:6114/udp \ # forwarding default d2cdb port
  -v /opt/pvpgn/etc:/usr/local/etc/pvpgn \ 
  -v /opt/pvpgn/var:/usr/local/var/pvpgn \ 
  pvpgn-server:d2dbs-mysql        # d2dbs with MySQL support image
```

#### To configurate MySQL connection:
To connect to MySQL inside the container, find the `storage_path` in `bnetd.conf` (in `/opt/pvpgn/etc`) and replace it with this:
```
storage_path = "sql:mode=mysql;host=mysql;name=bnetd;user=bnetd;pass=example;default=0;prefix=pvpgn_"
```

Some explanations:
* `host=mysql`: The host name we use which is also the name of the MySQL container
* `name=bnetd`, `user=bnetd`, `pass=example`: MySQL login info which is specified above

Change them to fit the MySQL server you created.

You may need to create multiple MySQL servers or multiple MySQL users for each servers or just simply change the table prefix from `pvpgn_` to something else.

#### Obtain logs

To obtain logs of the container, example to get `pvpgn-bnetd` log, run this command:

```bash
docker logs -f \ # view container logs continuously, ommit "-f" option if you don't want to keep seeing the logs
  --tail 1000 \  # get last 1000 lines of log ommit this to show all logs or decrease to 500 or 200 if you don't want to see too much log
  pvpgn-bnetd    # name of the container which is set above
```

Change `pvpgn-bnetd` to `pvpgn-d2cs` or `pvpgn-d2dbs` to get corresponding container log

### Using docker-compose:

To simplify the managing process, we should use `docker-compose`. The step-by-step example below will help you to create a system of 3 different containers running `bnetd`, `d2cs` and `d2dbs` with `mysql` storage:

1. Copy the sample `docker-compose.yml` file from this repository and modify it to suit you (like changing volume mounting):

2. Finally, hit `docker-compose up -d` to turn up all containers

#### To configurate MySQL connection:
To connect to MySQL inside the container, find the `storage_path` in `bnetd.conf` (in `/opt/pvpgn/etc`) and replace it with this:
```
storage_path = "sql:mode=mysql;host=mysql;name=bnetd;user=bnetd;pass=example;default=0;prefix=pvpgn_"
```

Some explanations:
* `host=mysql`: Name of the MySQL service which is configurated in `docker-compose.yml`.
* `name=bnetd`, `user=bnetd`, `pass=example`: MySQL login info which is also specified in 

Change them to fit the MySQL server you created.

You may need to create multiple MySQL ervers or multiple MySQL users for each servers or just simply change the table prefix from `pvpgn_` to something else.

#### Obtain logs

To obtain logs from containers managed by `docker-compose`, use this command:
```bash
cd /your/docker-compose-file/dir # move to the directory containing the docker-compose.yml file

docker-compose logs --tail 1000 # get last 1000 log lines of all containers managed by docker-compose
# or
docker-compose logs --tail 1000 bnetd # get last 1000 log lines of bnetd only
```
Check `docker-compose` specification for furter information.

## Some notes:

Here are some notes from my administration experience:

### Spin up a simple server:

Sometimes you just need a small server (for a small group of friends to play DotA) then just build pvpgn with sqlite support and mount the configuration files you want and let the rest unchanged:

```bash
# Prepare image with sqlite support
docker build . --build-arg with_sqlite3=true --build-arg with_mysql=false -t pvpgn-server:bnetd-sqlite

docker run -d \
  --name pvpgn-bnetd \
  -p 6112:6112 -p 6112:6112/udp \
  -v /opt/pvpgn/etc/bnetd.conf:/usr/local/etc/pvpgn/bnetd.conf \ # just mount the bnetd configuration files
  pvpgn-server:bnetd-sqlite
```

### Optimize networking performance:

Disable `userland-proxy` is recommended to run a big PVPGN server due to [performance degradation](https://franckpachot.medium.com/high-cpu-usage-in-docker-proxy-with-chatty-database-application-disable-userland-proxy-415ffa064955).

<div class="text-red mb-2">
    <h4>WARNING: After turning off `userland-proxy`, docker can manipulate your iptables without asking you. Use it with cares!</h4>
</div>

To achieve this, add `"userland-proxy": false` to `/etc/docker/daemon.json`:
```json
{
    "userland-proxy": false
    // your existing options
}
```

then restart docker daemon:
```bash
sudo systemctl restart docker
```

If `/etc/docker/daemon.json` file does not exist, just create a new one and add this to it then restart docker daemon with the command above:
```json
{
    "userland-proxy": false
}
```