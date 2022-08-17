# SLIME + DEEP LEARNING. 

    usage : slime-deep-learning > LICENSE
    usage : slime-deep-learning <cnf-instance>
    usage : slime-deep-learning <cnf-instance> <unsat-proof>
    usage : slime-deep-learning <cnf-instance> <unsat-proof> <sat-model>

example : slime-deep-learning -no-cryptography -learning-rate=0.001 -max-iter=10 instance.cnf instance.proof instance.model

  BOOST OPTIONS:

    -boost-alternate, -no-boost-alternate   (default: on)
    -boost, -no-boost                       (default: on)

  CORE OPTIONS:

    -gc-frac      = <double> (   0 ..  inf) (default: 0.2)
    -rinc         = <double> (   1 ..  inf) (default: 2)
    -cla-decay    = <double> (   0 ..    1) (default: 0.999)
    -var-decay    = <double> (   0 ..    1) (default: 0.8)
    -min-step-size = <double> (   0 ..    1) (default: 0.06)
    -step-size-dec = <double> (   0 ..    1) (default: 1e-06)
    -step-size    = <double> (   0 ..    1) (default: 0.4)

    -ccmin-mode   = <int32>  [   0 ..    2] (default: 2)
    -phase-saving = <int32>  [   0 ..    2] (default: 2)
    -rfirst       = <int32>  [   1 .. imax] (default: 100)
    -chrono       = <int32>  [  -1 .. imax] (default: 100)
    -confl-to-chrono = <int32>  [  -1 .. imax] (default: 4000)

  CRYPTOGRAPHY OPTIONS:

    -cryptography, -no-cryptography         (default: on)

  DEEP-LEARNING OPTIONS:

    -learning-rate = <double> (   0 ..  inf) (default: 0.1)

    -max-iter     = <int32>  [   0 .. imax] (default: 1)

  DISTANCE OPTIONS:

    -use-distance, -no-use-distance         (default: on)

  DUP-LEARNTS OPTIONS:

    -VSIDS-lim    = <int32>  [   1 .. imax] (default: 30)
    -min-dup-app  = <int32>  [   2 .. imax] (default: 2)
    -lbd-limit    = <int32>  [   0 .. imax] (default: 14)

  HESS OPTIONS:

    -hess, -no-hess                         (default: off)

    -hess-order   = <int32>  [   1 ..    2] (default: 1)

  LSIDS OPTIONS:

    -lsids-erase-weight = <double> [   0 ..    5] (default: 2)

  MASSIVE OPTIONS:

    -massive, -no-massive                   (default: off)

  POLARITY OPTIONS:

    -invert-polarity, -no-invert-polarity   (default: off)

  SIMP OPTIONS:

    -rcheck, -no-rcheck                     (default: off)
    -asymm, -no-asymm                       (default: off)
    -elim, -no-elim                         (default: on)

    -simp-gc-frac = <double> (   0 ..  inf) (default: 0.5)

    -cl-lim       = <int32>  [  -1 .. imax] (default: 20)
    -sub-lim      = <int32>  [  -1 .. imax] (default: 1000)
    -grow         = <int32>  [imin .. imax] (default: 0)

  HELP OPTIONS:

    --help        Print help message.
    --help-verb   Print verbose help message.
