(define (domain test-utils-domain)
  (:requirements :typing :negative-preconditions :strips)

  (:types t1 - object
          t2 - t1
          t3 - object)

  (:constants const0 - object
              const1 - t2)

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

  (:action a5
    :parameters (?a ?b ?c ?d)
    :precondition ()
    :effect (and (p1 ?a)
                 (p1 const0)
                 (not (p2 ?b ?c))
                 (not (p2 ?c ?d))
                 ;;; TODO: (not (p2 ?a ?a))
                 (not (p2 const0 ?a))))

  (:action a6
    :parameters (?a ?b ?c ?d)
    :precondition ()
    :effect (when (p1 ?a) (p1 ?b)))

  (:action a7
    :parameters (?a ?b ?c ?d)
    :precondition ()
    :effect (and (not (p2 ?a ?b))
                 ;;; (not (p2 ?b ?c)) (not (p2 ?c ?d))))
                   (when (p1 ?a) (and (not (p2 ?b ?c)) (not (p2 ?c ?d))))))

  (:action a8
    :parameters (?a ?b ?c ?d)
    :precondition ()
    :effect (forall (?e) (p1 ?e)))

  (:action a9
    :parameters (?a ?b ?c ?d)
    :precondition ()
    :effect (and (forall (?e) (and (p1 ?e) (p2 ?a ?e))) (p2 ?c ?d)))
)

