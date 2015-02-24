(define (domain asnlist)
  (:requirements :strips)

  (:predicates
    (p1 ?x)
    (p2 ?x ?y))

  (:action a1
    :parameters (?x ?y)
    :precondition (and (p1 ?x) (p1 ?y))
    :effect (p2 ?x ?y)))

