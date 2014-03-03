Testing:
Simplest testing enviroment requires at least:
* redis master
* redis slave
* redis sentinel
For example purpose master and slave will run on same machine and there are two separate machines running sentinel (quroum 2).

Configuration on master/slave machine:
diff -Eb /etc/redis/{redis.conf,redis_slave.conf} | grep "<\|>"
> 
< pidfile /var/run/redis/redis-master.pid
> pidfile /var/run/redis/redis-server2.pid
< port 6379
> port 6479
< logfile /var/log/redis/redis-master.log
> logfile /var/log/redis/redis-slave.log
< # slaveof <masterip> <masterport>
> slaveof 153.65.234.69 6379

Runing master/slave:
/usr/bin/redis-server /etc/redis/redis_slave.conf


Configuration on sentinel machines:
/etc/redis/sentinel.conf

port 26379
daemonize no
-change ip adress below-
sentinel monitor redis-master 153.65.234.69 6379 2
sentinel down-after-milliseconds redis-master 2000
sentinel can-failover redis-master yes
sentinel parallel-syncs redis-master 1
sentinel failover-timeout redis-master 900000
loglevel notice
logfile /var/log/redis/redis-sentinel.log

Runnig sentinel:
/usr/bin/redis-server /etc/redis/sentinel.conf --sentinel
