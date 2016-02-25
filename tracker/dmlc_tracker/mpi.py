"""
DMLC submission script, MPI version
"""
# pylint: disable=invalid-name
from __future__ import absolute_import

import os
import subprocess
from threading import Thread
from . import tracker

def submit(args):
    """Submission script with MPI."""
    # decide MPI version.
    (_, err) = subprocess.Popen('mpirun',
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE).communicate()
    if 'Open MPI' in err:
        mpi_version = 'openmpi'
    elif 'mpich' in err:
        mpi_version = 'mpich'
    else:
        raise RuntimeError('Unknown MPI Version')

    curr_path = os.path.dirname(os.path.abspath(os.path.expanduser(__file__)))
    launcher = os.path.join(curr_path, 'launcher.py')

    def mpi_submit(nworker, nserver, pass_envs):
        """Internal closure for job submission."""
        env = os.environ.copy()
        for k, v in pass_envs.items():
            env[k] = str(v)

        if args.mpi_host_file is None:
            cmd = 'mpirun -n %d' % (nworker + nserver)
        else:
            cmd = 'mpirun -n %d --hostfile %s ' % (nworker + nserver, args.mpi_host_file)

        pass_envs['DMLC_JOB_CLUSTER'] = 'mpi'
        pass_envs['DMLC_MPI_VERSION'] = mpi_version
        for k, v in pass_envs.items():
            # for mpich2
            if mpi_version == 'mpich':
                cmd += ' -env %s %s' % (k, v)
            else:
                cmd += ' -x %s=%s ' % (k, v)
        cmd += ' '
        cmd += ' '.join(args.command)

        thread = Thread(target=lambda: subprocess.check_call(cmd, shell=True, env=env), args=())
        thread.setDaemon(True)
        thread.start()

    tracker.submit(args.num_workers, args.num_servers,
                   fun_submit=mpi_submit,
                   pscmd=(' '.join(args.command)))
