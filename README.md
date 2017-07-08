Connect four with Monte Carlo method
===

This is one of the simplest implementation of a connect four AI you can write,
yet it is strong enough to play with humans showing to be a capable opponent.
The implementation uses the Monte Carlo methods class of algorithms: for every
potential move the computer can do, it does it and than plays random games,
sampling the number of games won and lost. Then the decision on what move to make
is made based on the move that appears to have the best won/lost ratio.

The code is very simple on purpose and every time there was to make a choice
between optimized and simple, the simple code was written, so there is space for
optimizations.
