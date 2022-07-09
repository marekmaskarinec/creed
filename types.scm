(module (creed types) 
	(make-location
		location-lno location-lno-set!
		location-cno location-cno-set!
		location-filename location-filename-set!

	make-token
		token-value token-value-set!
		token-type token-type-set!
		token-location token-location-set!

	make-state
		state-buffer state-buffer-set!
		state-mark state-mark-set!
		state-stack state-stack-set!
		state-symbols state-symbols-set!

	make-crerror)

	(import scheme (chicken base) (chicken format))


	(define-record location lno cno filename)
	(define-record-printer (location l out)
		(fprintf out "#,(location ~s:~s ~s)"
			(location-lno l)
			(location-cno l)
			(location-filename l)))

	(define-record token value type location)
	(define-record-printer (token t out)
		(fprintf out "#,(token ~s ~s ~s)"
			(token-value t)
			(token-type t)
			(token-location t)))

	(define-record state buffer mark stack symbols)

	(define-record crerror type location)
	(define-record-printer (crerror e out)
		(fprintf out "~s:(~s:~s) ~s"
			(location-filename (crerror-location e))
			(location-lno (crerror-location e))
			(location-cno (crerror-location e))
			(case (crerror-type e)
				((unterminated-group) "Unterminated group.")
				((unexpected-token) "Unexpected token.")))))
