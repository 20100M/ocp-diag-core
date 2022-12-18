"""
This module describes the test steps inside the test run.
"""
from contextlib import contextmanager
import typing as ty

from .objects import (
    StepArtifact,
    StepStart,
    StepEnd,
    Measurement,
    MeasurementValueType,
    Log,
    Error,
    File,
    Metadata,
    Diagnosis,
    TestStatus,
    LogSeverity,
    DiagnosisType,
)
from .measurement import MeasurementSeries, MeasurementSeriesEmitter, Validator
from .dut import Subcomponent, SoftwareInfo, HardwareInfo
from .output import ArtifactEmitter


class TestStepError(RuntimeError):
    __slots__ = "status"

    def __init__(self, *, status: TestStatus):
        self.status = status


class TestStep:
    """
    TODO:
    Should not be used directly by user code, but through TestRun.add_step().
    """

    def __init__(self, name: str, *, step_id: int, emitter: ArtifactEmitter):
        self._name = name
        self._id = step_id
        self._idstr = "{}".format(step_id)
        self._emitter = emitter

        # TODO: threadsafe?
        # TODO: do we want manually controlled values? lambda?
        self._measurement_series_id: int = 0

    def start(self):
        start = StepStart(name=self._name)
        self._emitter.emit(StepArtifact(id=self._idstr, impl=start))

    def end(self, *, status: TestStatus):
        end = StepEnd(status=status)
        self._emitter.emit(StepArtifact(id=self._idstr, impl=end))

    @contextmanager
    def scope(self):
        try:
            yield self.start()
        except TestStepError as e:
            self.end(status=e.status)
        else:
            self.end(status=TestStatus.COMPLETE)

    def add_measurement(
        self,
        *,
        name: str,
        value: MeasurementValueType,
        unit: ty.Optional[str] = None,
        validators: ty.Optional[list[Validator]] = None,
        hardware_info: ty.Optional[HardwareInfo] = None,
        subcomponent: ty.Optional[Subcomponent] = None,
        metadata: ty.Optional[Metadata] = None,
    ):
        if validators is None:
            validators = []

        measurement = Measurement(
            name=name,
            value=value,
            unit=unit,
            validators=[v.to_spec() for v in validators],
            hardware_info=hardware_info.to_spec() if hardware_info else None,
            subcomponent=subcomponent.to_spec() if subcomponent else None,
            metadata=metadata,
        )
        self._emitter.emit(StepArtifact(id=self._idstr, impl=measurement))

    def start_measurement_series(
        self,
        *,
        name: str,
        unit: ty.Optional[str] = None,
        validators: ty.Optional[list[Validator]] = None,
        hardware_info: ty.Optional[HardwareInfo] = None,
        subcomponent: ty.Optional[Subcomponent] = None,
        metadata: ty.Optional[Metadata] = None,
    ) -> MeasurementSeries:
        # TODO: arbitrary derivation for measurement id here, but unique for run scope
        series = MeasurementSeries(
            MeasurementSeriesEmitter(self._idstr, self._emitter),
            "{}_{}".format(self._id, self._measurement_series_id),
            name=name,
            unit=unit,
            validators=validators,
            hardware_info=hardware_info,
            subcomponent=subcomponent,
            metadata=metadata,
        )
        self._measurement_series_id += 1
        return series

    def add_diagnosis(
        self,
        diagnosis_type: DiagnosisType,
        *,
        verdict: str,
        message: ty.Optional[str] = None,
        hardware_info: ty.Optional[HardwareInfo] = None,
        subcomponent: ty.Optional[Subcomponent] = None,
    ):
        diag = Diagnosis(
            verdict=verdict,
            type=diagnosis_type,
            message=message,
            hardware_info=hardware_info.to_spec() if hardware_info else None,
            subcomponent=subcomponent.to_spec() if subcomponent else None,
        )
        self._emitter.emit(StepArtifact(id=self._idstr, impl=diag))

    def add_log(self, severity: LogSeverity, message: str):
        log = Log(
            severity=severity,
            message=message,
        )
        self._emitter.emit(StepArtifact(id=self._idstr, impl=log))

    def add_error(
        self,
        *,
        symptom: str,
        message: ty.Optional[str] = None,
        software_infos: ty.Optional[list[SoftwareInfo]] = None,
    ):
        if software_infos is None:
            software_infos = []

        error = Error(
            symptom=symptom,
            message=message,
            software_infos=[o.to_spec() for o in software_infos],
        )
        self._emitter.emit(StepArtifact(id=self._idstr, impl=error))

    def add_file(
        self,
        *,
        name: str,
        uri: str,
        is_snapshot: bool = True,
        description: ty.Optional[str] = None,
        content_type: ty.Optional[str] = None,
        metadata: ty.Optional[Metadata] = None,
    ):
        file = File(
            name=name,
            uri=uri,
            is_snapshot=is_snapshot,
            description=description,
            content_type=content_type,
            metadata=metadata,
        )
        self._emitter.emit(StepArtifact(id=self._idstr, impl=file))

    @property
    def name(self) -> str:
        return self._name

    @property
    def stepId(self) -> int:
        return self._id
