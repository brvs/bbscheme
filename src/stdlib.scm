
;; BoboScheme standard library
;;
;; Provides useful Scheme definitions that can be implemented in Scheme itself
;; to keep the C source as slim as possible.
;;
;; See src/bb_procedures.c to see functions provided in C.
;; Syntax exposed in C includes: 
;;    begin, cond, define, if, lambda, let, quote, set!

(define first car)
(define rest cdr)
(define null '())
(define else #t) ;; Hack. Remove when cond,case are macros

;; http://michaux.ca/articles/scheme-from-scratch-bootstrap-v0_13-lambda-the-ultimate
(define (map proc items)
  (if (null? items)
      '()
      (cons (proc (car items))
            (map proc (cdr items)))))

;; TODO: deal with (if #f "True")

(define %BB_FIXNUM 0)
(define %BB_BOOLEAN 1)
(define %BB_CHARACTER 2)
(define %BB_STRING 3)
(define %BB_SYMBOL 4)
(define %BB_EMPTY_LIST 5)
(define %BB_PAIR 6)
(define %BB_PRIMITIVE_PROCEDURE 7)
(define %BB_COMPOUND_PROCEDURE 8)
(define %BB_VOID 9)

(define (null? o)
  (= (%bb-object-type o) %BB_EMPTY_LIST))

(define (fixnum? o)
  (= (%bb-object-type o) %BB_FIXNUM))
(define number? fixnum?)

(define (boolean? o)
  (= (%bb-object-type o) %BB_BOOLEAN))

(define (character? o)
  (= (%bb-object-type o) %BB_CHARACTER))

(define (string? o)
  (= (%bb-object-type o) %BB_STRING))

(define (pair? o)
  (= (%bb-object-type o) %BB_PAIR))

(define (symbol? o)
  (= (%bb-object-type o) %BB_SYMBOL))

(define (procedure? o)
  (or (= (%bb-object-type o) %BB_PRIMITIVE_PROCEDURE)
      (= (%bb-object-type o) %BB_COMPOUND_PROCEDURE)))

(define (void? o)
  (= (%bb-object-type o) %BB_VOID))

(define (not x) (if x #f #t))


(define (%bb-display-pair obj)
  (%bb-display (car obj))
  (when (not (null? (cdr obj)))
    (if (pair? (cdr obj))
        (begin (printf " ")
               (%bb-display-pair (cdr obj)))
        (begin (printf " . ")
               (%bb-display (cdr obj))))))

;; TODO implicit (begin) in cond bodies
;; (define (display obj)
;;   (begin
;;     (cond
;;      ;; ((number? obj)
;;      ;;  (%bb-puts (number->string obj)))
;;      ;; ((boolean? obj)
;;      ;;  (%bb-puts (if obj "#t" "#f")))
;;      ;; ((string? obj)
;;      ;;  (%bb-puts obj))
;;      ;; ((symbol? obj)
;;      ;;  (%bb-puts (symbol->string obj)))
;;      ;; ((null? obj)
;;      ;;  (%bb-puts "()"))
;;      ;; ((pair? obj)
;;      ;;  (begin
;;      ;;    (%bb-puts "(")
;;      ;;    (%bb-display-pair obj)
;;      ;;    (%bb-puts ")")))
;;      ;; ((procedure?)
;;      ;;  (%bb-puts "<primitive proc>"))
;;      ;; ((void? obj)
;;      ;;  (void))
;;      ;; (else
;;      ;;  (%bb-puts "Unknown object type: ")))
;;     (void)))

;; TODO implicit (begin) in cond bodies
(define (display obj)
  (cond
   ;; ((number? obj)
   ;;  (%bb-puts (number->string obj)))
   ;; ((boolean? obj)
   ;;  (%bb-puts (if obj "#t" "#f")))
   ;; ((string? obj)
   ;;  (%bb-puts obj))
   ;; ((symbol? obj)
   ;;  (%bb-puts (symbol->string obj)))
   ;; ((null? obj)
   ;;  (%bb-puts "()"))
   ;; ((pair? obj)
   ;;  (begin
   ;;    (%bb-puts "(")
   ;;    (%bb-display-pair obj)
   ;;    (%bb-puts ")")))
   ;; ((procedure?)
   ;;  (%bb-puts "<primitive proc>"))
   ((void? obj)
    (void))
   (else
    (begin
      (%bb-puts "Unknown object type: ")
      (%bb-puts (number->string (%bb-object-type obj)))))))


'Standard-Library-Loaded
