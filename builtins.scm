(module (creed builtins)
	(builtins)

	(import scheme (chicken base) srfi-69)

	(define builtins (make-hash-table))

	(hash-table-set! builtins 'test-builtin
		(lambda (state token)
			(display "hello world\n"))))
