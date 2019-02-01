


(define (problem tsp-2)
(:domain tsp)
(:objects p1 p2 )
(:init
(at p1)
(connected p1 p2)
(connected p2 p1)
)
(:goal
(and
(visited p1)
(visited p2)
)
)
)


