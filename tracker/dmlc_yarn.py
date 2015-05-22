#!/usr/bin/env python
"""
This is a script to submit dmlc job via Yarn
dmlc will run as a Yarn application
"""
import argparse
import sys
import os
import time
import subprocess
import warnings
import tracker
import logging
from threading import Thread

YARN_JAR_PATH = os.path.dirname(__file__) + '/../yarn/dmlc-yarn.jar'
YARN_BOOT_PY = os.path.dirname(__file__) + '/../yarn/run_hdfs_prog.py'

if not os.path.exists(YARN_JAR_PATH):
    warnings.warn("cannot find \"%s\", I will try to run build" % YARN_JAR_PATH)
    cmd = 'cd %s;./build.sh' % (os.path.dirname(__file__) + '/../yarn/')
    print cmd
    subprocess.check_call(cmd, shell = True, env = os.environ) 
    assert os.path.exists(YARN_JAR_PATH), "failed to build dmlc-yarn.jar, try it manually"

hadoop_binary  = None
# code 
hadoop_home = os.getenv('HADOOP_HOME')

if hadoop_home != None:
    if hadoop_binary == None:
        hadoop_binary = hadoop_home + '/bin/hadoop'
        assert os.path.exists(hadoop_binary), "HADOOP_HOME does not contain the hadoop binary"


parser = argparse.ArgumentParser(description='Dmlc script to submit dmlc jobs to Yarn.')
parser.add_argument('-n', '--nworker', required=True, type=int,
                    help = 'number of worker proccess to be launched')
parser.add_argument('-hip', '--host_ip', default='auto', type=str,
                    help = 'host IP address if cannot be automatically guessed, specify the IP of submission machine')
parser.add_argument('-s', '--server-nodes', default = 0, type=int,
                    help = 'number of server nodes to be launched')
parser.add_argument('--log-level', default='INFO', type=str,
                    choices=['INFO', 'DEBUG'],
                    help = 'logging level')
parser.add_argument('--log-file', type=str,
                    help = 'output log to the specific log file')
parser.add_argument('-q', '--queue', default='default', type=str,
                    help = 'the queue we want to submit the job to')
parser.add_argument('-ac', '--auto_file_cache', default=1, choices=[0, 1], type=int,
                    help = 'whether automatically cache the files in the command to hadoop localfile, this is on by default')
parser.add_argument('-f', '--files', default = [], action='append',
                    help = 'the cached file list in mapreduce,'\
                        ' the submission script will automatically cache all the files which appears in command'\
                        ' This will also cause rewritten of all the file names in the command to current path,'\
                        ' for example `../../kmeans ../kmeans.conf` will be rewritten to `./kmeans kmeans.conf`'\
                        ' because the two files are cached to running folder.'\
                        ' You may need this option to cache additional files.'\
                        ' You can also use it to manually cache files when auto_file_cache is off')
parser.add_argument('--jobname', default='auto', help = 'customize jobname in tracker')
parser.add_argument('--tempdir', default='/tmp', help = 'temporary directory in HDFS that can be used to store intermediate results')
parser.add_argument('--vcores', default = 1, type=int,
                    help = 'number of vcpores to request in each mapper, set it if each dmlc job is multi-threaded')
parser.add_argument('-mem', '--memory_mb', default=1024, type=int,
                    help = 'maximum memory used by the process. Guide: set it large (near mapred.cluster.max.map.memory.mb)'\
                        'if you are running multi-threading dmlc,'\
                        'so that each node can occupy all the mapper slots in a machine for maximum performance')
parser.add_argument('--libhdfs-opts', default='-Xmx128m', type=str,
                    help = 'setting to be passed to libhdfs')
parser.add_argument('--name-node', default='default', type=str,
                    help = 'the namenode address of hdfs, libhdfs should connect to, normally leave it as default')
parser.add_argument('--ship-libcxx',default='none',type=str,
                    help = 'the path of gcc lib. if you change the default gcc version,you should ship the libstdc++.so or libstdc++.so.6 ') 
					
parser.add_argument('command', nargs='+',
                    help = 'command for dmlc program')
args = parser.parse_args()

if args.jobname == 'auto':
    args.jobname = ('DMLC[nworker=%d]:' % args.nworker) + args.command[0].split('/')[-1];

if hadoop_binary == None:
    parser.add_argument('-hb', '--hadoop_binary', required = True,
                        help="path to hadoop binary file")  
else:
    parser.add_argument('-hb', '--hadoop_binary', default = hadoop_binary, 
                        help="path to hadoop binary file")  

args = parser.parse_args()

if args.jobname == 'auto':
    if args.server_nodes == 0:
        args.jobname = ('DMLC[nworker=%d]:' % args.nworker) + args.command[0].split('/')[-1];
    else:
        args.jobname = ('DMLC[nworker=%d,nsever=%d]:' % (args.nworker, args.server_nodes)) + args.command[0].split('/')[-1];

# detech hadoop version
(out, err) = subprocess.Popen('%s version' % args.hadoop_binary, shell = True, stdout=subprocess.PIPE).communicate()
out = out.split('\n')[0].split()
assert out[0] == 'Hadoop', 'cannot parse hadoop version string'
hadoop_version = out[1].split('.')

(classpath, err) = subprocess.Popen('%s classpath --glob' % args.hadoop_binary, shell = True, stdout=subprocess.PIPE).communicate()

if hadoop_version < 2:    
    print 'Current Hadoop Version is %s, dmlc_yarn will need Yarn(Hadoop 2.0)' % out[1]

def yarn_submit(nworker, nserver, pass_env):
    """
      customized submit script, that submit nslave jobs, each must contain args as parameter
      note this can be a lambda function containing additional parameters in input
      Parameters
         nworker number of slave process to start up
         nserver number of server nodes to start up
         pass_env enviroment variables to be added to the starting programs
    """
    fset = set([YARN_JAR_PATH, YARN_BOOT_PY]) 
    flst = []
    if args.auto_file_cache != 0:
        for i in range(len(args.command)):
            f = args.command[i]
            if os.path.exists(f):
                fset.add(f)
                if i == 0:
                    args.command[i] = './' + args.command[i].split('/')[-1]
                else:
                    args.command[i] = './' + args.command[i].split('/')[-1]
        for f in flst:
            if os.path.exists(f):
                fset.add(f)            
	if args.ship_libcxx != 'none':
        sysbit = subprocess.Popen(['getconf LONG_BIT'],stdout=subprocess.PIPE,shell=True).communicate()
        libcxx = ''
        if sysbit[0] == '64\n':
                libcxx = args.ship_libcxx + '/libstdc++.so.6'
        else:
                libcxx = args.ship_libcxx + '/libstdc++.so'
        cmd = 'cp %s ./'%libcxx
        subprocess.check_call(cmd, shell = True)
        fset.add(libcxx)
	JAVA_HOME = os.getenv('JAVA_HOME')
    if JAVA_HOME is None:
        JAVA = 'java'
    else:
        JAVA = JAVA_HOME + '/bin/java'        
    cmd = '%s -cp `%s classpath`:%s org.apache.hadoop.yarn.dmlc.Client '\
          % (JAVA, args.hadoop_binary, YARN_JAR_PATH)
    env = os.environ.copy()
    for k, v in pass_env.items():
        env[k] = str(v)

    env['DMLC_CPU_VCORES'] = str(args.vcores)
    env['DMLC_MEMORY_MB'] = str(args.memory_mb)
    env['DMLC_NUM_WORKER'] = str(args.nworker)
    env['DMLC_NUM_SERVER'] = str(args.server_nodes)
    env['DMLC_HDFS_OPTS'] = str(args.libhdfs_opts)

    if args.files != None:
        for flst in args.files:
            for f in flst.split('#'):
                fset.add(f)
    for f in fset:
        cmd += ' -file %s' % f
    cmd += ' -jobname %s ' % args.jobname
    cmd += ' -tempdir %s ' % args.tempdir
    cmd += ' -queue %s ' % args.queue
    cmd += (' '.join(['./run_hdfs_prog.py'] + args.command))
    def run():
        logging.debug(cmd)
        subprocess.check_call(cmd, shell = True, env = env)
    thread = Thread(target = run, args=())
    thread.setDaemon(True)
    thread.start()    

tracker.config_logger(args)
tracker.submit(args.nworker, args.server_nodes, fun_submit = yarn_submit,
               pscmd= (' '.join([YARN_BOOT_PY] + args.command)))
