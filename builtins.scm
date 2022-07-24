(module (creed builtins)
	(builtins)

	(import
		scheme
		srfi-12
		srfi-69
		(chicken base)
		(creed types))

	(define builtins (make-hash-table))

	(define (register-builtin! symbol function)
		(hash-table-set! builtins symbol function))

	(define (pop! state loc)
		(if (null? (state-stack state))
			(abort (make-crerror 'stack-underflow loc))
			(begin
				(let ((val (car (state-stack state))))
					(state-stack-set! state (cdr (state-stack state)))
					val))))

	(define (pop-number! state loc)
		(let ((n (pop! state loc)))
			(if (number? n)
				n
				(abort (make-crerror 'type-error loc)))))

	(define (pop-group! state loc)
		(let ((n (pop! state loc)))
			(if (list? n)
				n
				(abort (make-crerror 'type-error loc)))))

	(define (pop-string! state loc)
		(let ((n (pop! state loc)))
			(if (string? n)
				n
				(abort (make-crerror 'type-error loc)))))

	(define (subst! state mark text)
		(let ((buf (state-buffer state)))

			(state-buffer-set! (string-append
				(substring buf 0 (car mark))
				text
				(substring buf (+ (car mark) (cdr mark)))))))

	(register-builtin! 'print
		(lambda (state loc)
			(display (pop! state loc)) (newline)))

	(register-builtin! 's
		(lambda (state loc)
			(subst! state (state-mark state) (pop-string! state loc))))

	(register-builtin! 'a
		(lambda (state loc)
			(let ((m (state-mark state)))
				(subst! state (cons (+ (car m) (cdr m)) 0)))))

	(register-builtin! 'p
		(lambda (state loc)
			(subst! state (cons (car (state-mark state)) 0))))


	)
