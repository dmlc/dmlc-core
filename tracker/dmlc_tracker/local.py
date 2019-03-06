"""Submission job for local jobs."""
# pylint: disable=invalid-name
from __future__ import absolute_import

import sys
import os
import subprocess
import logging
from threading import Thread
from . import tracker
import pdb

def exec_cmd(cmd, role, taskid, pass_env):
    """Execute the command line command."""
    if cmd[0].find('/') == -1 and os.path.exists(cmd[0]) and os.name != 'nt':
        cmd[0] = './' + cmd[0]
    cmdline = ' '.join(cmd)
    env = os.environ.copy()
    for k, v in pass_env.items():
        env[k] = str(v)

    env['DMLC_TASK_ID'] = str(taskid)
    env['DMLC_ROLE'] = role
    env['DMLC_JOB_CLUSTER'] = 'local'
    num_retry = env.get('DMLC_NUM_ATTEMPT', 0)

    #overwrite default num of retry with commandline value
    for parm in cmd:
        if parm.startswith('DMLC_NUM_ATTEMPT'):
            num_retry = int(parm.split('=')[1])
    logging.debug('num of retry %d',num_retry)

    while True:
        if os.name == 'nt':
            ret = subprocess.call(cmdline, shell=True, env=env)
        else:
            ret = subprocess.call(cmdline, shell=True, executable='bash', env=env)
        if ret == 0:
            logging.debug('Thread %d exit with 0', taskid)
            return
        else:
            num_retry -= 1
            newcmd = []
            if num_retry >= 0:
                # failure trail increase by 1 and restart failed worker
                for arg in cmd:
                    if arg.startswith('rabit_num_trial'):
                        val = arg.split('=')[1]
                        arg = arg.replace(val, str(int(val)+1))
                    newcmd.append(arg)
                cmdline = ' '.join(newcmd)
                cmd = newcmd
                continue
            if os.name == 'nt':
                sys.exit(-1)
            else:
                raise RuntimeError('Get nonzero return code=%d on %s' % (ret, cmd))


def submit(args):
    """Submit function of local jobs."""
    def mthread_submit(nworker, nserver, envs):
        """
        customized submit script, that submit nslave jobs, each must contain args as parameter
        note this can be a lambda function containing additional parameters in input

        Parameters
        ----------
        nworker: number of slave process to start up
        nserver: number of server nodes to start up
        envs: enviroment variables to be added to the starting programs
        """
        procs = {}
        for i in range(nworker + nserver):
            if i < nworker:
                role = 'worker'
            else:
                role = 'server'
            procs[i] = Thread(target=exec_cmd, args=(args.command, role, i, envs))
            procs[i].setDaemon(True)
            procs[i].start()

    # call submit, with nslave, the commands to run each job and submit function
    tracker.submit(args.num_workers, args.num_servers, fun_submit=mthread_submit,
                   pscmd=(' '.join(args.command)))
