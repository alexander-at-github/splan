


(define (problem tsp-15)
(:domain tsp)
(:objects p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 p13 p14 p15 )
(:init
(at p1)
(connected p1 p2)
(connected p2 p1)
(connected p1 p4)
(connected p4 p1)
(connected p1 p5)
(connected p5 p1)
(connected p1 p6)
(connected p6 p1)
(connected p1 p8)
(connected p8 p1)
(connected p1 p9)
(connected p9 p1)
(connected p1 p10)
(connected p10 p1)
(connected p1 p12)
(connected p12 p1)
(connected p1 p13)
(connected p13 p1)
(connected p3 p2)
(connected p2 p3)
(connected p3 p4)
(connected p4 p3)
(connected p3 p5)
(connected p5 p3)
(connected p3 p7)
(connected p7 p3)
(connected p3 p9)
(connected p9 p3)
(connected p3 p10)
(connected p10 p3)
(connected p3 p11)
(connected p11 p3)
(connected p3 p12)
(connected p12 p3)
(connected p3 p15)
(connected p15 p3)
(connected p5 p3)
(connected p3 p5)
(connected p5 p4)
(connected p4 p5)
(connected p5 p6)
(connected p6 p5)
(connected p5 p8)
(connected p8 p5)
(connected p5 p9)
(connected p9 p5)
(connected p5 p12)
(connected p12 p5)
(connected p5 p13)
(connected p13 p5)
(connected p5 p14)
(connected p14 p5)
(connected p7 p4)
(connected p4 p7)
(connected p7 p5)
(connected p5 p7)
(connected p7 p9)
(connected p9 p7)
(connected p7 p10)
(connected p10 p7)
(connected p7 p11)
(connected p11 p7)
(connected p7 p12)
(connected p12 p7)
(connected p7 p13)
(connected p13 p7)
(connected p7 p14)
(connected p14 p7)
(connected p7 p15)
(connected p15 p7)
(connected p9 p2)
(connected p2 p9)
(connected p9 p3)
(connected p3 p9)
(connected p9 p5)
(connected p5 p9)
(connected p9 p10)
(connected p10 p9)
(connected p11 p2)
(connected p2 p11)
(connected p11 p3)
(connected p3 p11)
(connected p11 p6)
(connected p6 p11)
(connected p11 p7)
(connected p7 p11)
(connected p11 p10)
(connected p10 p11)
(connected p11 p12)
(connected p12 p11)
(connected p11 p15)
(connected p15 p11)
(connected p13 p3)
(connected p3 p13)
(connected p13 p4)
(connected p4 p13)
(connected p13 p7)
(connected p7 p13)
(connected p13 p9)
(connected p9 p13)
(connected p13 p12)
(connected p12 p13)
(connected p13 p15)
(connected p15 p13)
(connected p15 p2)
(connected p2 p15)
(connected p15 p5)
(connected p5 p15)
(connected p15 p8)
(connected p8 p15)
(connected p15 p9)
(connected p9 p15)
(connected p15 p12)
(connected p12 p15)
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
(visited p12)
(visited p13)
(visited p14)
(visited p15)
)
)
)


