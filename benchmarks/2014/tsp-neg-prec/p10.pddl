


(define (problem tsp-11)
(:domain tsp)
(:objects p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 )
(:init
(at p1)
(connected p1 p3)
(connected p3 p1)
(connected p1 p5)
(connected p5 p1)
(connected p1 p8)
(connected p8 p1)
(connected p1 p10)
(connected p10 p1)
(connected p1 p11)
(connected p11 p1)
(connected p3 p2)
(connected p2 p3)
(connected p3 p8)
(connected p8 p3)
(connected p3 p9)
(connected p9 p3)
(connected p3 p10)
(connected p10 p3)
(connected p5 p2)
(connected p2 p5)
(connected p5 p3)
(connected p3 p5)
(connected p5 p8)
(connected p8 p5)
(connected p5 p9)
(connected p9 p5)
(connected p5 p10)
(connected p10 p5)
(connected p5 p11)
(connected p11 p5)
(connected p7 p2)
(connected p2 p7)
(connected p7 p3)
(connected p3 p7)
(connected p7 p4)
(connected p4 p7)
(connected p7 p6)
(connected p6 p7)
(connected p7 p8)
(connected p8 p7)
(connected p7 p9)
(connected p9 p7)
(connected p7 p10)
(connected p10 p7)
(connected p7 p11)
(connected p11 p7)
(connected p9 p2)
(connected p2 p9)
(connected p9 p3)
(connected p3 p9)
(connected p9 p5)
(connected p5 p9)
(connected p9 p8)
(connected p8 p9)
(connected p11 p6)
(connected p6 p11)
(connected p11 p8)
(connected p8 p11)
(connected p11 p9)
(connected p9 p11)
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
(visited p8)
(visited p9)
(visited p10)
(visited p11)
)
)
)


