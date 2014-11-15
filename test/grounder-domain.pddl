(define (domain test-grounder-domain)
  (:requirements :typing :negative-preconditions :strips)
  (:types t1 - object
          t2 - t1
          t3 - object)
  (:constants const1 - t1)

  (:predicates (p1 ?a ?b)
               (p2 ?a)
               (p3 ?a ?b ?c))

  (:action a1
    :parameters (?a - t1
                 ?b - t3
                 ?c)
    :precondition (and (p1 ?a ?b) (p2 ?c))
    :effect ()))
