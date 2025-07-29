from fractions import Fraction
from typing import Callable, List, Optional

from unified_planning.shortcuts import Problem
from unified_planning.engines.results import LogMessage

from tyr.planners.model.apptainer_planner import ApptainerPlanner
from tyr.planners.model.pddl_planner import TyrPDDLPlanner


# pylint: disable=too-many-ancestors
class TflapPlanner(ApptainerPlanner):
    """The TFlap planner wrapped into local apptainer planner."""
    
    def __init__(self, needs_requirements=True, rewrite_bool_assignments=False) -> None:
        super().__init__(needs_requirements, rewrite_bool_assignments)
        self._computation_time: Optional[float] = None

    def _file_extension(self) -> str:
        return "pddl"

    def _get_apptainer_file_name(self) -> str:
        return "tflap.sif"
    
    def _get_computation_time(self, _logs: List[LogMessage]) -> Optional[float]:
        return self._computation_time

    def _get_engine_epsilon(self) -> Fraction:
        return Fraction(1, 1000)

    # def _get_write_domain_options(self) -> Dict[str, bool]:
    #     opts = super()._get_write_domain_options()
    #     opts["control_support"] = True
    #     opts["force_predicate"] = True
    #     return opts
    
    def _plan_from_str(self, problem: Problem, plan_str: str, get_item_named: Callable):
        lines = plan_str.split("\n")
        plan: List[str] = []
        parsing = True
        for line in lines:
            if line.strip() == "":
                continue
            if line.startswith(self._ending_plan_str()):
                parsing = False
                continue
            if parsing:
                plan.append(self._parse_plan_line(line))
                continue
            if line.startswith(";Total time"):
                self._computation_time = float(line.split(":")[1].strip())
                continue

        if plan:
            self._plan_found = True
        return TyrPDDLPlanner._plan_from_str(self, problem, "\n".join(plan), get_item_named)

    def _starting_plan_str(self) -> str:
        return ""

    def _ending_plan_str(self) -> str:
        return ";Makespan"

    def _parse_plan_line(self, plan_line: str) -> str:
        return plan_line


__all__ = ["TflapPlanner"]
