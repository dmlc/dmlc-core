"""Job submission script"""

from __future__ import absolute_import

import logging

from . import kubernetes, local, mesos, mpi, opts, sge, yarn


def config_logger(args):
    """Configure the logger according to the arguments

    Parameters
    ----------
    args: argparser.Arguments
       The arguments passed in by the user.
    """
    fmt = "%(asctime)s %(levelname)s %(message)s"
    if args.log_level == "INFO":
        level = logging.INFO
    elif args.log_level == "DEBUG":
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
        logging.getLogger("").addHandler(console)


def main():
    """Main submission function."""
    args = opts.get_opts()
    config_logger(args)

    if args.cluster == "local":
        local.submit(args)
    elif args.cluster == "sge":
        sge.submit(args)
    elif args.cluster == "yarn":
        yarn.submit(args)
    elif args.cluster == "mpi":
        mpi.submit(args)
    elif args.cluster == "mesos":
        mesos.submit(args)
    elif args.cluster == "kubernetes":
        kubernetes.submit(args)
    else:
        raise RuntimeError("Unknown submission cluster type %s" % args.cluster)
