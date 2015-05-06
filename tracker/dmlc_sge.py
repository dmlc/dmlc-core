#!/usr/bin/env python
"""
Submit rabit jobs to Sun Grid Engine
"""
import argparse
import sys
import os
import tracker
import subprocess

parser = argparse.ArgumentParser(description='DMLC script to submit dmlc job using Sun Grid Engine')
parser.add_argument('-n', '--nworker', required=True, type=int,
                    help = 'number of worker proccess to be launched')
parser.add_argument('-s', '--server-nodes', default = 0, type=int,
                    help = 'number of server nodes to be launched')
parser.add_argument('-q', '--queue', default='default', type=str,
                    help = 'the queue we want to submit the job to')
parser.add_argument('--log-level', default='INFO', type=str,
                    choices=['INFO', 'DEBUG'],
                    help = 'logging level')
parser.add_argument('--log-file', type=str,
                    help = 'output log to the specific log file')
parser.add_argument('--logdir', default='auto', help = 'customize the directory to place the SGE job logs')
parser.add_argument('-hip', '--host_ip', default='auto', type=str,
                    help = 'host IP address if cannot be automatically guessed, specify the IP of submission machine')
parser.add_argument('--vcores', default = 1, type=int,
                    help = 'number of vcpores to request in each mapper, set it if each dmlc job is multi-threaded')
parser.add_argument('--jobname', default='auto', help = 'customize jobname in tracker')
parser.add_argument('command', nargs='+',
                    help = 'command for dmlc program')
args, unknown = parser.parse_known_args()

if args.jobname == 'auto':
    args.jobname = ('dmlc%d.' % args.nworker) + args.command[0].split('/')[-1];
if args.logdir == 'auto':
    args.logdir = args.jobname + '.log'

if os.path.exists(args.logdir):
    if not os.path.isdir(args.logdir):
        raise RuntimeError('specified logdir %s is a file instead of directory' % args.logdir)
else:
    os.mkdir(args.logdir)
    
runscript = '%s/rundmlc.sh' % args.logdir
fo = open(runscript, 'w')
fo.write('source ~/.bashrc\n')
fo.write('export DMLC_TASK_ID=${SGE_TASK_ID}\n')
fo.write('\"$@\"\n')
fo.close()

def sge_submit(nworker, nserver, pass_envs):
    """
      customized submit script, that submit nslave jobs, each must contain args as parameter
      note this can be a lambda function containing additional parameters in input
      Parameters
         nworker number of slave process to start up
         nserver number of server nodes to start up
         pass_envs enviroment variables to be added to the starting programs
    """
    env_arg = ','.join(['%s=\"%s\"' % (k, str(v)) for k, v in pass_envs.items()])    
    cmd = 'qsub -cwd -t 1-%d -S /bin/bash' % (nworker + nserver)
    if args.queue != 'default':
        cmd += '-q %s' % args.queue
    cmd += ' -N %s ' % args.jobname
    cmd += ' -e %s -o %s' % (args.logdir, args.logdir)
    cmd += ' -pe orte %d' % (args.vcores)
    cmd += ' -v %s,PATH=${PATH}:.' % env_arg
    cmd += ' %s %s' % (runscript, ' '.join(args.command) + ' ' + ' '.join(unknown))
    print cmd
    subprocess.check_call(cmd, shell = True)
    print 'Waiting for the jobs to get up...'

tracker.config_logger(args)
# call submit, with nslave, the commands to run each job and submit function
tracker.submit(args.nworker, args.server_nodes, fun_submit = sge_submit,
               pscmd=(' '.join(args.command) + ' ' + ' '.join(unknown)))

