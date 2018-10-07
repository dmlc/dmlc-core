"""Submission job for local jobs."""
# pylint: disable=invalid-name
from __future__ import absolute_import

import sys
import os
import subprocess
import logging
from threading import Thread
from . import tracker
from .opts import parse_env_pairs

def prepare_envs(args):
    """
    Load environment variables from arguments
    """
    envs = {}
    # envs['default'] refers to the default env variables passed for both worker and server

    # if --env option was used to pass a variable it would automatically be
    # included because of the next line
    envs['default'] = os.environ.copy()

    # given by user
    common_envs = [item for item in args.env.split(',') if item]
    for k in common_envs:
        if k not in envs['default']:
            # raise warning if user intended to set this
            logging.info('The environment variable %s was passed but not set', k)

    envs['server'] = parse_env_pairs(args.env_server)
    envs['worker'] = parse_env_pairs(args.env_worker)
    return envs

def exec_cmd(cmd, role, taskid, dmlc_envs, addnl_envs):
    """Execute the command line command."""
    if cmd[0].find('/') == -1 and os.path.exists(cmd[0]) and os.name != 'nt':
        cmd[0] = './' + cmd[0]
    cmd = ' '.join(cmd)
    env = addnl_envs['default'].copy()
    for k in dmlc_envs:
        env[k] = str(dmlc_envs[k])

    if role == 'worker':
        env.update(addnl_envs['worker'])
    else:
        env.update(addnl_envs['server'])

    env['DMLC_TASK_ID'] = str(taskid)
    env['DMLC_ROLE'] = role
    env['DMLC_JOB_CLUSTER'] = 'local'

    num_retry = env.get('DMLC_NUM_ATTEMPT', 0)

    while True:
        if os.name == 'nt':
            ret = subprocess.call(cmd, shell=True, env=env)
        else:
            ret = subprocess.call(cmd, shell=True, executable='bash', env=env)
        if ret == 0:
            logging.debug('Thread %d exit with 0', taskid)
            return
        else:
            num_retry -= 1
            if num_retry >= 0:
                continue
            if os.name == 'nt':
                sys.exit(-1)
            else:
                raise RuntimeError('Get nonzero return code=%d on %s %s' % (ret, cmd, env))

def submit(args):
    """Submit function of local jobs."""
    envs = prepare_envs(args)

    def mthread_submit(nworker, nserver, dmlc_envs, addnl_envs):
        """
        customized submit script, that submit nslave jobs, each must contain args as parameter
        note this can be a lambda function containing additional parameters in input

        Parameters
        ----------
        nworker: number of slave process to start up
        nserver: number of server nodes to start up
        dmlc_envs: refers to the environment variables required to start distributed training
        addnl_envs: are optional environment variables passed by the user
        """
        procs = {}
        for i in range(nworker + nserver):
            if i < nworker:
                role = 'worker'
            else:
                role = 'server'
            procs[i] = Thread(target=exec_cmd, args=(args.command, role, i, dmlc_envs, addnl_envs))
            procs[i].setDaemon(True)
            procs[i].start()

    # call submit, with nslave, the commands to run each job and submit function
    tracker.submit(args.num_workers, args.num_servers, fun_submit=mthread_submit,
                   pscmd=(' '.join(args.command)),
                   addnl_envs=envs)
