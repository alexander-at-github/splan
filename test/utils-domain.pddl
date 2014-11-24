(define (domain test-utils-domain)
  (:requirements :typing :negative-preconditions :strips)

  (:types t1 - object
          t2 - t1
          t3 - object)

  (:constants const1 - t1)

  (:predicates (p1 ?a)
               (p2 ?a ?b)
               (p3 ?a ?b ?c))

  ;;; Simple example
  (:action a0
    :parameters (?a)
    :precondition ()
    :effect (p1 ?a))

  (:action a1
    :parameters (?a ?b)
    :precondition ()
    :effect (and (p1 ?a) (p2 ?a ?b)))

  (:action a2
    :parameters (?a ?b)
    :precondition ()
    :effect (and (p1 ?a) (p2 ?a ?b)))

  (:action a3
    :parameters (?a ?b ?c ?d)
    :precondition ()
    :effect (and (p1 ?a) (p2 ?a ?b) (p2 ?b ?a) (p3 ?a ?c ?d) (p3 ?c ?d ?c)))

  (:action a4
    :parameters (?a ?b ?c ?d)
    :precondition ()
    :effect (not (p2 ?a ?c)))
)

