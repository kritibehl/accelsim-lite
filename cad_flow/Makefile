.PHONY: lint sim impact regression clean

lint:
	python3 scripts/run_lint.py

sim:
	python3 scripts/run_sim.py

impact:
	python3 scripts/changed_rtl_impact.py

regression:
	cd .. && python3 cad_flow/scripts/run_regression.py

clean:
	rm -f reports/*.json reports/*.md
