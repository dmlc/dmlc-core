#!/usr/bin/env python
"""
DMLC submission script by ssh

One need to make sure all slaves machines are ssh-able.
"""
from __future__ import absolute_import

from multiprocessing import Pool, Process
import os, subprocess, logging
from threading import Thread
from . import tracker
from .opts import parse_env_pairs

def sync_dir(local_dir, slave_node, slave_dir):
    """
    sync the working directory from root node into slave node
    """
    remote = slave_node[0] + ':' + slave_dir
    logging.info('rsync %s -> %s', local_dir, remote)
    prog = 'rsync -az --rsh="ssh -o StrictHostKeyChecking=no -p %s" %s %s' % (
        slave_node[1], local_dir, remote)
    subprocess.check_call([prog], shell=True)

def prepare_envs(args):
    """
    Load environment variables from arguments
    """
    envs = {}
    # default env variables which are always passed by the system
    envs['default'] = {'OMP_NUM_THREADS', 'KMP_AFFINITY', 'LD_LIBRARY_PATH', 'AWS_ACCESS_KEY_ID',
                          'AWS_SECRET_ACCESS_KEY', 'DMLC_INTERFACE'}

    # given by user with the option --env
    envs['user'] = set([item for item in args.env.split(',') if item])
    # remove those specified by user from default so we can confirm that user has set these vars
    envs['default'].difference_update(envs['user'])
    envs['server'] = parse_env_pairs(args.env_server)
    envs['worker'] = parse_env_pairs(args.env_worker)
    return envs

def get_envs_str(dmlc_envs, addnl_envs, is_server):
    """
    Returns a string containing the command to export all relevant environment variables
    :param dmlc_envs: required environment variables for distributed training
    :param addnl_envs: environment variables which user might want and those specified explicitly
    :param is_server: whether these variables are for a server node
    :return: the export command string
    """
    # appends a pair of key and value to the list representing export command
    def append_env_var(envs_list, key, value):
        envs_list.append('export ' + key + '=' + str(value) + ';')

    # add role to dmlc_envs
    dmlc_envs['DMLC_ROLE'] = 'server' if is_server else 'worker'

    envs = []
    for k in addnl_envs['default']:
        v = os.getenv(k)
        if v is not None:
            append_env_var(envs, k, v)

    for k in addnl_envs['user']:
        v = os.getenv(k)
        if v is not None:
            append_env_var(envs, k, v)
        else:
            # ensure user didn't make an error
            raise ValueError('The environment variable '+ k + ' was passed but not set')
    if is_server:
        for k, v in addnl_envs['server'].items():
            append_env_var(envs, k, v)
    else:
        for k, v in addnl_envs['worker'].items():
            append_env_var(envs, k, v)

    # required dmlc_envs
    for k, v in dmlc_envs.items():
        append_env_var(envs, k, v)
    return (' '.join(envs))

def prepare_hosts(host_file):
    """
    return list of tuples of host_ip and port
    """
    assert host_file is not None
    with open(host_file) as f:
        tmp = f.readlines()
    assert len(tmp) > 0
    hosts=[]
    for h in tmp:
        if len(h.strip()) > 0:
            # parse addresses of the form ip:port
            h = h.strip()
            i = h.find(":")
            p = "22"
            if i != -1:
                p = h[i+1:]
                h = h[:i]
            # hosts now contain the pair ip, port
            hosts.append((h, p))
    return hosts

def submit(args):
    hosts = prepare_hosts(args.host_file)
    envs = prepare_envs(args)

    def ssh_submit(nworker, nserver, dmlc_envs, addnl_envs):
        """
        customized submit script
        """
        # thread func to run the job
        def run(prog):
            subprocess.check_call(prog, shell = True)

        # sync programs if necessary
        local_dir = os.getcwd()+'/'
        working_dir = local_dir
        if args.sync_dst_dir is not None and args.sync_dst_dir != 'None':
            working_dir = args.sync_dst_dir
            pool = Pool(processes=len(hosts))
            for h in hosts:
                pool.apply_async(sync_dir, args=(local_dir, h, working_dir))
            pool.close()
            pool.join()
            

        # launch jobs
        for i in range(nworker + nserver):
            is_server = i < nserver
            (node, port) = hosts[i % len(hosts)]
            dmlc_envs['DMLC_NODE_HOST'] = node
            prog = get_envs_str(dmlc_envs, addnl_envs, is_server)
            prog += ' cd ' + working_dir + '; ' + (' '.join(args.command))
            prog = 'ssh -o StrictHostKeyChecking=no ' + node + ' -p ' + port + ' \'' + prog + '\''
            thread = Thread(target=run, args=(prog,))
            thread.setDaemon(True)
            thread.start()
        return ssh_submit

    tracker.submit(args.num_workers, args.num_servers,
                   fun_submit=ssh_submit,
                   pscmd=(' '.join(args.command)),
                   hostIP=args.host_ip,
                   addnl_envs=envs)
