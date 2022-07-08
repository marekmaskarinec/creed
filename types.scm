(module (creed types) 
	(make-location
		location-lno location-lno-set!
		location-cno location-cno-set!
		location-filename location-filename-set!

	make-token
		token-value token-value-set!
		token-type token-type-set!
		token-location token-location-set!)

	(import (chicken base) (chicken format))


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

	(define-record state buffer mark stack symbols))
