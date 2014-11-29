(define (domain test-utils-domain)
  (:requirements :typing :negative-preconditions :strips)

  (:types t1 - object
          t2 - t1
          t3 - t2)

  (:constants const0 - object
              const1 - t1)

  (:predicates (p1 ?a)
               (p2 ?a ?b)
               (p3 ?a ?b ?c))


  ;;; Actions for testing the search for gaps.

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

  ;;; Actions for testing the grounding based on gaps

  (:action a10
    :parameters (?a - object)
    :precondition ()
    :effect (p1 ?a))

  (:action a11
    :parameters (?a ?b)
    :precondition ()
    :effect (p2 ?a ?b))

  ;;; Type hierarchie is like that: 't2' extends 't1' extends 'object'
  (:action a12
    :parameters (?a - t1 ?b - t2)
    :precondition ()
    :effect (p2 ?a ?b))

  ;;; Type hierarchie is like that: 't2' extends 't1' extends 'object'
  (:action a13
    :parameters (?a ?b - t2)
    :precondition ()
    :effect (p2 ?a ?b))

  (:action a14
    :parameters (?a - t1)
    :precondition ()
    :effect (p1 ?a))

  (:action a15
    :parameters (?a ?b - t2)
    :precondition ()
    :effect (p2 ?a ?b))

  (:action a16
    :parameters (?a - t1
                 ?b - t2
                 ?c - t3
                 ?d - object)
    :precondition ()
    :effect (and (p1 ?a)
                 ;;; should match in test_actionFixesGap_advanced_typed()
                 (p2 ?a ?b)
                 ;;; should match in test_actionFixesGap_advanced_typed()
                 (p2 ?d ?b)
                 ;;; should not match in test_actionFixesGap_advanced_typed()
                 (p2 ?b ?a)
                 ;;; should not match in test_actionFixesGap_advanced_typed()
                 (p2 ?a ?c)
            )
  )
)

