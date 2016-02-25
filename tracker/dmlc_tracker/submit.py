"""Job submission script"""
from __future__ import absolute_import

import logging
from . import opts
from . import local
from . import mpi
from . import sge
from . import yarn

def config_logger(args):
    """Configure the logger according to the arguments

    Parameters
    ----------
    args: argparser.Arguments
       The arguments passed in by the user.
    """
    fmt = '%(asctime)s %(levelname)s %(message)s'
    if args.log_level == 'INFO':
        level = logging.INFO
    elif args.log_level == 'DEBUG':
        level = logging.DEBUG
    else:
        raise RuntimeError("Unknown logging level %s" % args.log_level)

    if args.log_file is None:
        logging.basicConfig(format=fmt, level=level)
    else:
        logging.basicConfig(format=fmt, level=level, filename=args.log_file)
        console = logging.StreamHandler()
        console.setFormatter(logging.Formatter(fmt))
        console.setLevel(level)
        logging.getLogger('').addHandler(console)

def main():
    """Main submission function."""
    args = opts.get_opts()
    config_logger(args)

    if args.mode == 'local':
        local.submit(args)
    elif args.mode == 'sge':
        sge.submit(args)
    elif args.mode == 'yarn':
        yarn.submit(args)
    elif args.mode == 'mpi':
        mpi.submit(args)
    else:
        raise RuntimeError('Unknown submission mode %s' % args.mode)
