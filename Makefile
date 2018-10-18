pre:
	cd freedom-e-sdk/software && ln -sd ../../sevensegment

upload:
	cd freedom-e-sdk && make software PROGRAM=sevensegment && sudo make upload PROGRAM=sevensegment
