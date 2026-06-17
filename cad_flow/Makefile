.PHONY: lint verilator sim icarus yosys impact dashboard regression clean

lint:
	python3 scripts/run_lint.py

verilator:
	cd .. && python3 cad_flow/scripts/run_verilator_lint.py

sim:
	python3 scripts/run_sim.py

icarus:
	cd .. && python3 cad_flow/scripts/run_icarus_sim.py

yosys:
	cd .. && python3 cad_flow/scripts/run_yosys_check.py

impact:
	cd .. && python3 cad_flow/scripts/changed_rtl_impact.py

dashboard:
	cd .. && python3 cad_flow/scripts/build_dashboard.py

regression:
	cd .. && python3 cad_flow/scripts/run_regression.py

clean:
	rm -f reports/*.json reports/*.md
	rm -rf reports/icarus_build
