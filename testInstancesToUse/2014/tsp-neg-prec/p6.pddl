(define (problem tsp-7)
(:domain tsp)
(:objects p1 p2 p3 p4 p5 p6 p7 )
(:init
(at p1)
(connected p1 p4)
(connected p4 p1)
(connected p1 p2)
(connected p2 p1)
(connected p3 p2)
(connected p2 p3)
(connected p3 p5)
(connected p5 p3)
(connected p3 p6)
(connected p6 p3)
(connected p3 p7)
(connected p7 p3)
(connected p5 p7)
(connected p7 p5)
(connected p7 p6)
(connected p6 p7)
)
(:goal
(and
(visited p1)
(visited p2)
(visited p3)
(visited p4)
(visited p5)
(visited p6)
(visited p7)
)
)
)

