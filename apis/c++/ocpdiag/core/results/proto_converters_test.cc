// Copyright 2022 Google LLC
//
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include "ocpdiag/core/results/proto_converters.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/string_view.h"
#include "ocpdiag/core/results/dut_info.h"
#include "ocpdiag/core/results/results.pb.h"
#include "ocpdiag/core/results/structs.h"
#include "ocpdiag/core/testing/parse_text_proto.h"
#include "ocpdiag/core/testing/proto_matchers.h"

using ::ocpdiag::testing::EqualsProto;
using ::ocpdiag::testing::ParseTextProtoOrDie;
using ::ocpdiag::testing::Partially;

namespace ocpdiag::results::internal {

Subcomponent GetExampleSubcomponent() {
  return {
      .name = "FAN1",
      .type = SubcomponentType::kUnspecified,
      .location = "F0_1",
      .version = "1",
      .revision = "1",
  };
}

RegisteredHardwareInfo GetRegisteredHardwareInfo() {
  DutInfo dut_info("dut", "id");
  return dut_info.AddHardwareInfo({.name = "hw_info"});
}

RegisteredSoftwareInfo GetRegisteredSoftwareInfo() {
  DutInfo dut_info("dut", "id");
  return dut_info.AddSoftwareInfo({.name = "sw_info"});
}

TEST(StructToProtoTest, MeasurementSeriesStartStructConvertsSuccessfully) {
  MeasurementSeriesStart measurement_series_start = {
      .name = "measured-fan-speed-100",
      .unit = "RPM",
      .hardware_info = GetRegisteredHardwareInfo(),
      .subcomponent = GetExampleSubcomponent(),
      .validators = {{
                         .type = ValidatorType::kLessThanOrEqual,
                         .value = {11000.0},
                         .name = "80mm_fan_upper_limit",
                     },
                     {
                         .type = ValidatorType::kGreaterThanOrEqual,
                         .value = {8000.0},
                         .name = "80mm_fan_lower_limit",
                     }},
      .metadata_json = R"json({"some": "JSON"})json",
  };
  EXPECT_THAT(StructToProto(measurement_series_start), EqualsProto(R"pb(
                name: "measured-fan-speed-100"
                unit: "RPM"
                hardware_info_id: "0"
                subcomponent {
                  name: "FAN1"
                  location: "F0_1"
                  version: "1"
                  revision: "1"
                  type: UNSPECIFIED
                }
                validators {
                  name: "80mm_fan_upper_limit"
                  type: LESS_THAN_OR_EQUAL
                  value: { number_value: 11000.0 }
                }
                validators {
                  name: "80mm_fan_lower_limit"
                  type: GREATER_THAN_OR_EQUAL
                  value: { number_value: 8000.0 }
                }
                metadata {
                  fields {
                    key: "some"
                    value { string_value: "JSON" }
                  }
                }
              )pb"));
}

TEST(StructToProtoTest, MeasurementSeriesElementStructConvertsSuccessfully) {
  MeasurementSeriesElement measurement_series_element = {
      .value = 123.,
      .timestamp = timeval{.tv_sec = 100, .tv_usec = 15},
      .metadata_json = R"json({"some": "JSON"})json",
  };
  EXPECT_THAT(StructToProto(measurement_series_element), EqualsProto(R"pb(
                value: { number_value: 123.0 }
                timestamp { seconds: 100 nanos: 15000 }
                metadata {
                  fields {
                    key: "some"
                    value { string_value: "JSON" }
                  }
                }
              )pb"));
}

TEST(StructToProtoTest, MeasurementStructConvertsSuccessfully) {
  Measurement measurement = {
      .name = "measured-fan-speed-100",
      .unit = "RPM",
      .hardware_info = GetRegisteredHardwareInfo(),
      .subcomponent = GetExampleSubcomponent(),
      .validators = {{
                         .type = ValidatorType::kLessThanOrEqual,
                         .value = {11000.0},
                         .name = "80mm_fan_upper_limit",
                     },
                     {
                         .type = ValidatorType::kGreaterThanOrEqual,
                         .value = {8000.0},
                         .name = "80mm_fan_lower_limit",
                     }},
      .value = 100.,
      .metadata_json = R"json({"some": "JSON"})json",
  };
  EXPECT_THAT(StructToProto(measurement), EqualsProto(R"pb(
                name: "measured-fan-speed-100"
                unit: "RPM"
                hardware_info_id: "0"
                subcomponent {
                  name: "FAN1"
                  location: "F0_1"
                  version: "1"
                  revision: "1"
                  type: UNSPECIFIED
                }
                validators {
                  name: "80mm_fan_upper_limit"
                  type: LESS_THAN_OR_EQUAL
                  value: { number_value: 11000.0 }
                }
                validators {
                  name: "80mm_fan_lower_limit"
                  type: GREATER_THAN_OR_EQUAL
                  value: { number_value: 8000.0 }
                }
                value { number_value: 100 }
                metadata {
                  fields {
                    key: "some"
                    value { string_value: "JSON" }
                  }
                }
              )pb"));
}

TEST(StructToProtoTest, StringValidatorConvertsSuccessfully) {
  Measurement measurement = {.name = "string-test",
                             .validators = {{
                                 .type = ValidatorType::kEqual,
                                 .value = {"Test", "value"},
                             }},
                             .value = "string"};
  EXPECT_THAT(StructToProto(measurement), EqualsProto(R"pb(
                name: "string-test"
                validators {
                  type: EQUAL
                  value {
                    list_value {
                      values { string_value: "Test" }
                      values { string_value: "value" }
                    }
                  }
                }
                value { string_value: "string" }
                metadata {}
              )pb"));
}

TEST(StructToProtoTest, BoolValidatorConvertsSuccessfully) {
  Measurement measurement = {.name = "bool-test",
                             .validators = {{
                                 .type = ValidatorType::kEqual,
                                 .value = {true},
                             }},
                             .value = false};
  EXPECT_THAT(StructToProto(measurement), EqualsProto(R"pb(
                name: "bool-test"
                validators {
                  type: EQUAL
                  value: { bool_value: True }
                }
                value { bool_value: False }
                metadata {}
              )pb"));
}

TEST(StructToProtoTest, DiagnosisStructConvertsSuccessfully) {
  RegisteredHardwareInfo hw_info = GetRegisteredHardwareInfo();
  Diagnosis diagnosis = {
      .verdict = "mlc-intranode-bandwidth-pass",
      .type = DiagnosisType::kPass,
      .message = "intranode bandwidth within threshold.",
      .hardware_info = GetRegisteredHardwareInfo(),
      .subcomponent = Subcomponent({
          .name = "QPI1",
          .type = SubcomponentType::kBus,
          .location = "CPU-3-2-3",
          .version = "1",
          .revision = "0",
      }),
  };
  EXPECT_THAT(StructToProto(diagnosis), EqualsProto(R"pb(
                verdict: "mlc-intranode-bandwidth-pass"
                type: PASS
                message: "intranode bandwidth within threshold."
                hardware_info_id: "0"
                subcomponent {
                  type: BUS
                  name: "QPI1"
                  location: "CPU-3-2-3"
                  version: "1"
                  revision: "0"
                }
              )pb"));
}

TEST(StructToProtoTest, ErrorStructConvertsSuccessfully) {
  Error error = {
      .symptom = "bad-return-code",
      .message = "software exited abnormally.",
      .software_infos = {GetRegisteredSoftwareInfo()},
  };
  EXPECT_THAT(StructToProto(error), EqualsProto(R"pb(
                symptom: "bad-return-code"
                message: "software exited abnormally."
                software_info_ids: "0"
              )pb"));
}

TEST(StructToProtoTest, FileStructConvertsSuccessfully) {
  File file = {
      .display_name = "mem_cfg_log",
      .uri = "file:///root/mem_cfg_log",
      .is_snapshot = false,
      .description = "DIMM configuration settings.",
      .content_type = "text/plain",
  };
  EXPECT_THAT(StructToProto(file), EqualsProto(R"pb(
                display_name: "mem_cfg_log"
                uri: "file:///root/mem_cfg_log"
                description: "DIMM configuration settings."
                content_type: "text/plain"
                is_snapshot: false
              )pb"));
}

TEST(StructToProtoTest, TestRunStartStructConvertsSuccessfully) {
  TestRunStart test_run_start = {
      .name = "mlc_test",
      .version = "1.0",
      .command_line =
          "mlc/mlc --use_default_thresholds=true --data_collection_mode=true",
      .parameters_json = R"json({
        "max_bandwidth": 7200.0,
        "mode": "fast_mode",
        "data_collection_mode": true,
        "min_bandwidth": 700.0,
        "use_default_thresholds": true
      })json",
      .metadata_json = R"json({"some": "JSON"})json",
  };
  EXPECT_THAT(
      StructToProto(test_run_start), EqualsProto(R"pb(
        name: "mlc_test"
        version: "1.0"
        command_line: "mlc/mlc --use_default_thresholds=true --data_collection_mode=true"
        parameters {
          fields {
            key: "data_collection_mode"
            value { bool_value: true }
          }
          fields {
            key: "max_bandwidth"
            value { number_value: 7200 }
          }
          fields {
            key: "min_bandwidth"
            value { number_value: 700 }
          }
          fields {
            key: "mode"
            value { string_value: "fast_mode" }
          }
          fields {
            key: "use_default_thresholds"
            value { bool_value: true }
          }
        }
        metadata {
          fields {
            key: "some"
            value { string_value: "JSON" }
          }
        }
      )pb"));
}

TEST(StructToProtoTest, LogStructConvertsSuccessfully) {
  Log log = {
      .severity = LogSeverity::kError,
      .message = "file operation not completed successfully.",
  };
  EXPECT_THAT(StructToProto(log), EqualsProto(R"pb(
                severity: ERROR
                message: "file operation not completed successfully."
              )pb"));
}

TEST(StructToProtoTest, ExtensionStructConvertsSuccessfully) {
  Extension extension = {
      .name = "Extension",
      .content_json = R"json({"some": "JSON"})json",
  };
  EXPECT_THAT(StructToProto(extension), EqualsProto(R"pb(
                name: "Extension"
                content {
                  fields {
                    key: "some"
                    value { string_value: "JSON" }
                  }
                }
              )pb"));
}

TEST(JsonToProtoTest, ValidJsonConvertsSuccessfully) {
  absl::string_view valid_json = R"json({
    "A field": "with a value",
    "An object": {"Another field": "another value"},
    "A list": ["with", "values"]
  })json";
  EXPECT_THAT(JsonToProtoOrDie(valid_json), EqualsProto(R"pb(
                fields {
                  key: "A field"
                  value { string_value: "with a value" }
                }
                fields {
                  key: "A list"
                  value {
                    list_value {
                      values { string_value: "with" }
                      values { string_value: "values" }
                    }
                  }
                }
                fields {
                  key: "An object"
                  value {
                    struct_value {
                      fields {
                        key: "Another field"
                        value { string_value: "another value" }
                      }
                    }
                  }
                }
              )pb"));
}

TEST(JsonToProtoTest, EmptyJsonCreatesEmpty) {
  EXPECT_THAT(JsonToProtoOrDie(""), EqualsProto(""));
}

TEST(JsonToProtoDeathTest, InvalidJsonCausesError) {
  absl::string_view invalid_json = R"json({
    "You forgot a comma": "in this"
    "json": "string"
  })json";
  EXPECT_DEATH(JsonToProtoOrDie(invalid_json), "");
}

TEST(DutInfoToProtoTest, DutInfoConvertsSuccessfully) {
  DutInfo dut_info("dut", "id");
  RegisteredHardwareInfo hw_info1 = dut_info.AddHardwareInfo({
      .name = "primary node",
      .computer_system = "primary_node",
      .location = "MB/DIMM_A1",
      .odata_id = "/redfish/v1/Systems/System.Embedded.1/Memory/DIMMSLOTA1",
      .part_number = "P03052-091",
      .serial_number = "HMA2022029281901",
      .manager = "bmc0",
      .manufacturer = "hynix",
      .manufacturer_part_number = "HMA84GR7AFR4N-VK",
      .part_type = "DIMM",
      .version = "1",
      .revision = "2",
  });
  RegisteredSoftwareInfo hw_info2 = dut_info.AddSoftwareInfo({
      .name = "bmc_firmware",
      .computer_system = "primary_node",
      .version = "1",
      .revision = "2",
      .software_type = SoftwareType::kFirmware,
  });
  dut_info.AddPlatformInfo({.info = "memory_optimized"});
  dut_info.SetMetadataJson(R"json({"some": "JSON"})json");

  EXPECT_THAT(
      DutInfoToProto(dut_info), Partially(EqualsProto(R"pb(
        name: "dut"
        dut_info_id: "id"
        metadata {
          fields {
            key: "some"
            value { string_value: "JSON" }
          }
        }
        platform_infos { info: "memory_optimized" }
        hardware_infos {
          computer_system: "primary_node"
          name: "primary node"
          location: "MB/DIMM_A1"
          odata_id: "/redfish/v1/Systems/System.Embedded.1/Memory/DIMMSLOTA1"
          part_number: "P03052-091"
          serial_number: "HMA2022029281901"
          manager: "bmc0"
          manufacturer: "hynix"
          manufacturer_part_number: "HMA84GR7AFR4N-VK"
          part_type: "DIMM"
          version: "1"
          revision: "2"
        }
        software_infos {
          computer_system: "primary_node"
          name: "bmc_firmware"
          version: "1"
          revision: "2"
          software_type: FIRMWARE
        }
      )pb")));
}

TEST(ProtoToStructTest, SchemaVersionProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    schema_version { major: 2 minor: 0 }
  )pb");
  EXPECT_EQ(ProtoToStruct(proto),
            OutputArtifact(
                {.artifact = SchemaVersionOutput({.major = 2, .minor = 0})}));
}

TEST(ProtoToStructTest, TestRunStartProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(
      R"pb(
        test_run_artifact {
          test_run_start {
            name: "mlc_test"
            version: "1.0"
            command_line: "mlc/mlc --use_default_thresholds=true --data_collection_mode=true"
            parameters {
              fields {
                key: "use_default_thresholds"
                value { bool_value: true }
              }
            }
            dut_info {
              dut_info_id: "mydut"
              name: "dut"
              platform_infos { info: "memory_optimized" }
              hardware_infos {
                hardware_info_id: "1"
                computer_system: "primary_node"
                name: "primary node"
                location: "MB/DIMM_A1"
                odata_id: "/redfish/v1/Systems/System.Embedded.1/Memory/DIMMSLOTA1"
                part_number: "P03052-091"
                serial_number: "HMA2022029281901"
                manager: "bmc0"
                manufacturer: "hynix"
                manufacturer_part_number: "HMA84GR7AFR4N-VK"
                part_type: "DIMM"
                version: "1"
                revision: "2"
              }
              software_infos {
                software_info_id: "1"
                computer_system: "primary_node"
                name: "bmc_firmware"
                version: "1"
                revision: "2"
                software_type: FIRMWARE
              }
              metadata {
                fields {
                  key: "some"
                  value { string_value: "JSON" }
                }
              }
            }
            metadata {
              fields {
                key: "some"
                value { string_value: "JSON" }
              }
            }
          }
        }
      )pb");

  EXPECT_EQ(
      ProtoToStruct(proto),
      OutputArtifact(
          {.artifact = TestRunArtifact(
               {.artifact = TestRunStartOutput({
                    .name = "mlc_test",
                    .version = "1.0",
                    .command_line = "mlc/mlc --use_default_thresholds=true "
                                    "--data_collection_mode=true",
                    .parameters_json =
                        R"json({"use_default_thresholds":true})json",
                    .dut_info =
                        {
                            .dut_info_id = "mydut",
                            .name = "dut",
                            .metadata_json = R"json({"some":"JSON"})json",
                            .platform_infos = {{.info = "memory_optimized"}},
                            .hardware_infos = {{
                                .hardware_info_id = "1",
                                .name = "primary node",
                                .computer_system = "primary_node",
                                .location = "MB/DIMM_A1",
                                .odata_id =
                                    "/redfish/v1/Systems/System.Embedded.1/"
                                    "Memory/DIMMSLOTA1",
                                .part_number = "P03052-091",
                                .serial_number = "HMA2022029281901",
                                .manager = "bmc0",
                                .manufacturer = "hynix",
                                .manufacturer_part_number = "HMA84GR7AFR4N-VK",
                                .part_type = "DIMM",
                                .version = "1",
                                .revision = "2",
                            }},
                            .software_infos = {{
                                .software_info_id = "1",
                                .name = "bmc_firmware",
                                .computer_system = "primary_node",
                                .version = "1",
                                .revision = "2",
                                .software_type = SoftwareType::kFirmware,
                            }},
                        },
                    .metadata_json = R"json({"some":"JSON"})json",
                })})}));
}

TEST(ProtoToStructTest, TestRunEndProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_run_artifact { test_run_end { status: COMPLETE result: PASS } }
  )pb");
  EXPECT_EQ(ProtoToStruct(proto),
            OutputArtifact({.artifact = TestRunArtifact(
                                {.artifact = TestRunEndOutput(
                                     {.status = TestStatus::kComplete,
                                      .result = TestResult::kPass})})}));
}

TEST(ProtoToStructTest, LogProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_run_artifact {
      log {
        severity: ERROR
        message: "file operation not completed successfully."
      }
    }
  )pb");
  EXPECT_EQ(
      ProtoToStruct(proto),
      OutputArtifact(
          {.artifact = TestRunArtifact(
               {.artifact = LogOutput({
                    .severity = LogSeverity::kError,
                    .message = "file operation not completed successfully.",
                })})}));
}

TEST(ProtoToStructTest, ErrorProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_run_artifact {
      error {
        symptom: "bad-return-code"
        message: "software exited abnormally."
        software_info_ids: "1"
        software_info_ids: "2"
      }
    }
  )pb");
  EXPECT_EQ(ProtoToStruct(proto),
            OutputArtifact({.artifact = TestRunArtifact(
                                {.artifact = ErrorOutput({
                                     .symptom = "bad-return-code",
                                     .message = "software exited abnormally.",
                                     .software_info_ids = {"1", "2"},
                                 })})}));
}

TEST(ProtoToStructTest, TestStepStartProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_step_artifact { test_step_start { name: "my step" } }
  )pb");
  EXPECT_EQ(ProtoToStruct(proto),
            OutputArtifact(
                {.artifact = TestStepArtifact(
                     {.artifact = TestStepStartOutput({.name = "my step"})})}));
}

TEST(ProtoToStructTest, TestStepEndProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_step_artifact { test_step_end { status: ERROR } }
  )pb");
  EXPECT_EQ(ProtoToStruct(proto),
            OutputArtifact({.artifact = TestStepArtifact(
                                {.artifact = TestStepEndOutput(
                                     {.status = TestStatus::kError})})}));
}

TEST(ProtoToStructTest, MeasurementProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_step_artifact {
      measurement {
        name: "measured-fan-speed-100"
        unit: "RPM"
        hardware_info_id: "5"
        subcomponent {
          name: "FAN1"
          location: "F0_1"
          version: "1"
          revision: "1"
          type: UNSPECIFIED
        }
        validators {
          name: "Fan name"
          type: EQUAL
          value: { string_value: "My fan name" }
        }
        value { string_value: "My fan name" }
        metadata {
          fields {
            key: "some"
            value { string_value: "JSON" }
          }
        }
      }
    }
  )pb");
  EXPECT_EQ(
      ProtoToStruct(proto),
      OutputArtifact({.artifact = TestStepArtifact(
                          {.artifact = MeasurementOutput({
                               .name = "measured-fan-speed-100",
                               .unit = "RPM",
                               .hardware_info_id = "5",
                               .subcomponent = GetExampleSubcomponent(),
                               .validators = {{
                                   .type = ValidatorType::kEqual,
                                   .value = {"My fan name"},
                                   .name = "Fan name",
                               }},
                               .value = "My fan name",
                               .metadata_json = R"json({"some":"JSON"})json",
                           })})}));
}

TEST(ProtoToStructTest, MeasurementSeriesStartProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_step_artifact {
      measurement_series_start {
        measurement_series_id: "13"
        name: "measured-fan-speed-100"
        unit: "RPM"
        hardware_info_id: "5"
        subcomponent {
          name: "FAN1"
          location: "F0_1"
          version: "1"
          revision: "1"
          type: UNSPECIFIED
        }
        validators {
          name: "80mm_fan_upper_limit"
          type: LESS_THAN_OR_EQUAL
          value: { number_value: 11000.0 }
        }
        validators {
          name: "80mm_fan_lower_limit"
          type: GREATER_THAN_OR_EQUAL
          value: { number_value: 8000.0 }
        }
        metadata {
          fields {
            key: "some"
            value { string_value: "JSON" }
          }
        }
      }
    }
  )pb");
  EXPECT_EQ(
      ProtoToStruct(proto),
      OutputArtifact(
          {.artifact = TestStepArtifact(
               {.artifact = MeasurementSeriesStartOutput({
                    .measurement_series_id = "13",
                    .name = "measured-fan-speed-100",
                    .unit = "RPM",
                    .hardware_info_id = "5",
                    .subcomponent = GetExampleSubcomponent(),
                    .validators = {{
                                       .type = ValidatorType::kLessThanOrEqual,
                                       .value = {11000.},
                                       .name = "80mm_fan_upper_limit",
                                   },
                                   {
                                       .type =
                                           ValidatorType::kGreaterThanOrEqual,
                                       .value = {8000.},
                                       .name = "80mm_fan_lower_limit",
                                   }},
                    .metadata_json = R"json({"some":"JSON"})json",
                })})}));
}

TEST(ProtoToStructTest, MeasurementSeriesElementProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_step_artifact {
      measurement_series_element {
        index: 144
        measurement_series_id: "12"
        value { number_value: 100219.0 }
        timestamp { seconds: 1000 nanos: 150000 }
        metadata {
          fields {
            key: "some"
            value { string_value: "JSON" }
          }
        }
      }
    }
  )pb");
  EXPECT_EQ(
      ProtoToStruct(proto),
      OutputArtifact({.artifact = TestStepArtifact(
                          {.artifact = MeasurementSeriesElementOutput({
                               .index = 144,
                               .measurement_series_id = "12",
                               .value = 100219.,
                               .timestamp =
                                   {
                                       .tv_sec = 1000,
                                       .tv_usec = 150,
                                   },
                               .metadata_json = R"json({"some":"JSON"})json",
                           })})}));
}

TEST(ProtoToStructTest, MeasurementSeriesEndProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_step_artifact {
      measurement_series_end { measurement_series_id: "3" total_count: 51 }
    }
  )pb");
  EXPECT_EQ(ProtoToStruct(proto),
            OutputArtifact({.artifact = TestStepArtifact(
                                {.artifact = MeasurementSeriesEndOutput({
                                     .measurement_series_id = "3",
                                     .total_count = 51,
                                 })})}));
}

TEST(ProtoToStructTest, DiagnosisProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_step_artifact {
      diagnosis {
        verdict: "mlc-intranode-bandwidth-pass"
        type: PASS
        message: "intranode bandwidth within threshold."
        hardware_info_id: "10"
        subcomponent {
          type: BUS
          name: "QPI1"
          location: "CPU-3-2-3"
          version: "1"
          revision: "0"
        }
      }
    }
  )pb");
  EXPECT_EQ(ProtoToStruct(proto),
            OutputArtifact(
                {.artifact = TestStepArtifact(
                     {.artifact = DiagnosisOutput({
                          .verdict = "mlc-intranode-bandwidth-pass",
                          .type = DiagnosisType::kPass,
                          .message = "intranode bandwidth within threshold.",
                          .hardware_info_id = "10",
                          .subcomponent = Subcomponent({
                              .name = "QPI1",
                              .type = SubcomponentType::kBus,
                              .location = "CPU-3-2-3",
                              .version = "1",
                              .revision = "0",
                          }),
                      })})}));
}

TEST(ProtoToStructTest, FileProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_step_artifact {
      file {
        display_name: "mem_cfg_log"
        uri: "file:///root/mem_cfg_log"
        description: "DIMM configuration settings."
        content_type: "text/plain"
        is_snapshot: false
      }
    }
  )pb");
  EXPECT_EQ(
      ProtoToStruct(proto),
      OutputArtifact({.artifact = TestStepArtifact(
                          {.artifact = FileOutput({
                               .display_name = "mem_cfg_log",
                               .uri = "file:///root/mem_cfg_log",
                               .is_snapshot = false,
                               .description = "DIMM configuration settings.",
                               .content_type = "text/plain",
                           })})}));
}

TEST(ProtoToStructTest, ExtensionProtoConvertsSuccessfully) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_step_artifact {
      extension {
        name: "Extension"
        content {
          fields {
            key: "some"
            value { string_value: "JSON" }
          }
        }
      }
    }
  )pb");
  EXPECT_EQ(
      ProtoToStruct(proto),
      OutputArtifact({.artifact = TestStepArtifact(
                          {.artifact = ExtensionOutput({
                               .name = "Extension",
                               .content_json = R"json({"some":"JSON"})json",
                           })})}));
}

TEST(ProtoToStructTest, OutputArtifactFieldsAreSetProperlyDuringConversion) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    schema_version { major: 2 minor: 0 }
    sequence_number: 3
    timestamp { seconds: 101 nanos: 102000 }
  )pb");
  EXPECT_EQ(
      ProtoToStruct(proto),
      OutputArtifact({.artifact = SchemaVersionOutput({.major = 2, .minor = 0}),
                      .sequence_number = 3,
                      .timestamp = {.tv_sec = 101, .tv_usec = 102}}));
}

TEST(ProtoToStructTest, TestStepFieldsAreSetProperlyDuringConversion) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_step_artifact {
      log { severity: ERROR message: "Fake error message" }
      test_step_id: "4"
    }
  )pb");
  EXPECT_EQ(ProtoToStruct(proto),
            OutputArtifact(
                {.artifact = TestStepArtifact(
                     {.artifact = LogOutput({.severity = LogSeverity::kError,
                                             .message = "Fake error message"}),
                      .test_step_id = "4"})}));
}

TEST(ProtoToStructTest, ErrorProtoConvertsSuccessfullyForTestStep) {
  ocpdiag_results_v2_pb::OutputArtifact proto = ParseTextProtoOrDie(R"pb(
    test_step_artifact {
      error {
        symptom: "internal-error"
        message: "fake"
        software_info_ids: "1"
        software_info_ids: "2"
      }
      test_step_id: "7"
    }
  )pb");
  EXPECT_EQ(
      ProtoToStruct(proto),
      OutputArtifact(
          {.artifact = TestStepArtifact(
               {.artifact = ErrorOutput({.symptom = "internal-error",
                                         .message = "fake",
                                         .software_info_ids = {"1", "2"}}),
                .test_step_id = "7"})}));
}

TEST(ProtoToStructDeathTest, EmptyOutputArtifactDies) {
  ocpdiag_results_v2_pb::OutputArtifact proto;
  EXPECT_DEATH(ProtoToStruct(proto), "empty or unexepected OutputArtifact");
}

TEST(ProtoToStructDeathTest, EmptyTestRunArtifactDies) {
  ocpdiag_results_v2_pb::OutputArtifact proto =
      ParseTextProtoOrDie(R"pb(test_run_artifact {})pb");
  EXPECT_DEATH(ProtoToStruct(proto), "empty or unexepected TestRunArtifact");
}

TEST(ProtoToStructDeathTest, EmptyTestStepArtifactDies) {
  ocpdiag_results_v2_pb::OutputArtifact proto =
      ParseTextProtoOrDie(R"pb(test_step_artifact {})pb");
  EXPECT_DEATH(ProtoToStruct(proto), "empty or unexepected TestStepArtifact");
}

TEST(ProtoToJsonOrDie, ValidProtoConvertsSuccessfully) {
  google::protobuf::Struct proto = ParseTextProtoOrDie(R"pb(
    fields {
      key: "data_collection_mode"
      value { bool_value: true }
    }
  )pb");

  EXPECT_EQ(ProtoToJsonOrDie(proto),
            R"json({"data_collection_mode":true})json");
}

}  // namespace ocpdiag::results::internal
