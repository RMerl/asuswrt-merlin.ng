// Copyright (c) 2016-2019, The Tor Project, Inc. */
// See LICENSE for licensing information */

extern crate protover;

use protover::errors::ProtoverError;
use protover::ProtoEntry;
use protover::ProtoverVote;
use protover::UnvalidatedProtoEntry;

#[test]
fn parse_protocol_with_single_proto_and_single_version() {
    let _: ProtoEntry = "Cons=1".parse().unwrap();
}

#[test]
fn parse_protocol_with_single_protocol_and_multiple_versions() {
    let _: ProtoEntry = "Cons=1-2".parse().unwrap();
}

#[test]
fn parse_protocol_with_different_single_protocol_and_single_version() {
    let _: ProtoEntry = "HSDir=1".parse().unwrap();
}

#[test]
fn parse_protocol_with_single_protocol_and_supported_version() {
    let _: ProtoEntry = "Desc=2".parse().unwrap();
}

#[test]
fn parse_protocol_with_two_protocols_and_single_version() {
    let _: ProtoEntry = "Cons=1 HSDir=1".parse().unwrap();
}

#[test]
fn parse_protocol_with_single_protocol_and_two_sequential_versions() {
    let _: ProtoEntry = "Desc=1-2".parse().unwrap();
}

#[test]
fn parse_protocol_with_single_protocol_and_protocol_range() {
    let _: ProtoEntry = "Link=1-4".parse().unwrap();
}

#[test]
fn parse_protocol_with_single_protocol_and_protocol_set() {
    let _: ProtoEntry = "Link=3-4 Desc=2".parse().unwrap();
}

#[test]
fn protocol_all_supported_with_single_protocol_and_protocol_set() {
    let protocols: UnvalidatedProtoEntry = "Link=3-4 Desc=2".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocols.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protocol_all_supported_with_two_values() {
    let protocols: UnvalidatedProtoEntry = "Microdesc=1-2 Relay=2".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocols.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protocol_all_supported_with_one_value() {
    let protocols: UnvalidatedProtoEntry = "Microdesc=1-2".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocols.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protocol_all_supported_with_three_values() {
    let protocols: UnvalidatedProtoEntry = "LinkAuth=1 Microdesc=1-2 Relay=2".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocols.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protocol_all_supported_with_unsupported_protocol() {
    let protocols: UnvalidatedProtoEntry = "Wombat=9".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocols.all_supported();
    assert_eq!(true, unsupported.is_some());
    assert_eq!("Wombat=9", &unsupported.unwrap().to_string());
}

#[test]
fn protocol_all_supported_with_unsupported_versions() {
    let protocols: UnvalidatedProtoEntry = "Link=3-63".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocols.all_supported();
    assert_eq!(true, unsupported.is_some());
    assert_eq!("Link=6-63", &unsupported.unwrap().to_string());
}

#[test]
fn protocol_all_supported_with_unsupported_low_version() {
    let protocols: UnvalidatedProtoEntry = "HSIntro=2-3".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocols.all_supported();
    assert_eq!(true, unsupported.is_some());
    assert_eq!("HSIntro=2", &unsupported.unwrap().to_string());
}

#[test]
fn protocol_all_supported_with_unsupported_high_version() {
    let protocols: UnvalidatedProtoEntry = "Cons=1-2,60".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocols.all_supported();
    assert_eq!(true, unsupported.is_some());
    assert_eq!("Cons=60", &unsupported.unwrap().to_string());
}

#[test]
fn protocol_all_supported_with_mix_of_supported_and_unsupproted() {
    let protocols: UnvalidatedProtoEntry = "Link=3-4 Wombat=9".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocols.all_supported();
    assert_eq!(true, unsupported.is_some());
    assert_eq!("Wombat=9", &unsupported.unwrap().to_string());
}

#[test]
fn protover_string_supports_protocol_returns_true_for_single_supported() {
    let protocols: UnvalidatedProtoEntry = "Link=3-4 Cons=1".parse().unwrap();
    let is_supported = protocols.supports_protocol(&protover::Protocol::Cons.into(), &1);
    assert_eq!(true, is_supported);
}

#[test]
fn protover_string_supports_protocol_returns_false_for_single_unsupported() {
    let protocols: UnvalidatedProtoEntry = "Link=3-4 Cons=1".parse().unwrap();
    let is_supported = protocols.supports_protocol(&protover::Protocol::Cons.into(), &2);
    assert_eq!(false, is_supported);
}

#[test]
fn protover_string_supports_protocol_returns_false_for_unsupported() {
    let protocols: UnvalidatedProtoEntry = "Link=3-4".parse().unwrap();
    let is_supported = protocols.supports_protocol(&protover::Protocol::Cons.into(), &2);
    assert_eq!(false, is_supported);
}

#[test]
#[should_panic]
fn parse_protocol_with_unexpected_characters() {
    let _: UnvalidatedProtoEntry = "Cons=*-%".parse().unwrap();
}

#[test]
fn protover_compute_vote_returns_empty_for_empty_string() {
    let protocols: &[UnvalidatedProtoEntry] = &["".parse().unwrap()];
    let listed = ProtoverVote::compute(protocols, &1);
    assert_eq!("", listed.to_string());
}

#[test]
fn protover_compute_vote_returns_single_protocol_for_matching() {
    let protocols: &[UnvalidatedProtoEntry] = &["Cons=1".parse().unwrap()];
    let listed = ProtoverVote::compute(protocols, &1);
    assert_eq!("Cons=1", listed.to_string());
}

#[test]
fn protover_compute_vote_returns_two_protocols_for_two_matching() {
    let protocols: &[UnvalidatedProtoEntry] = &["Link=1 Cons=1".parse().unwrap()];
    let listed = ProtoverVote::compute(protocols, &1);
    assert_eq!("Cons=1 Link=1", listed.to_string());
}

#[test]
fn protover_compute_vote_returns_one_protocol_when_one_out_of_two_matches() {
    let protocols: &[UnvalidatedProtoEntry] =
        &["Cons=1 Link=2".parse().unwrap(), "Cons=1".parse().unwrap()];
    let listed = ProtoverVote::compute(protocols, &2);
    assert_eq!("Cons=1", listed.to_string());
}

#[test]
fn protover_compute_vote_returns_protocols_that_it_doesnt_currently_support() {
    let protocols: &[UnvalidatedProtoEntry] =
        &["Foo=1 Cons=2".parse().unwrap(), "Bar=1".parse().unwrap()];
    let listed = ProtoverVote::compute(protocols, &1);
    assert_eq!("Bar=1 Cons=2 Foo=1", listed.to_string());
}

#[test]
fn protover_compute_vote_returns_matching_for_mix() {
    let protocols: &[UnvalidatedProtoEntry] = &["Link=1-10,50 Cons=1,3-7,8".parse().unwrap()];
    let listed = ProtoverVote::compute(protocols, &1);
    assert_eq!("Cons=1,3-8 Link=1-10,50", listed.to_string());
}

#[test]
fn protover_compute_vote_returns_matching_for_longer_mix() {
    let protocols: &[UnvalidatedProtoEntry] = &[
        "Desc=1-10,50 Cons=1,3-7,8".parse().unwrap(),
        "Link=12-45,8 Cons=2-6,8 Desc=9".parse().unwrap(),
    ];

    let listed = ProtoverVote::compute(protocols, &1);
    assert_eq!("Cons=1-8 Desc=1-10,50 Link=8,12-45", listed.to_string());
}

#[test]
fn protover_compute_vote_returns_matching_for_longer_mix_with_threshold_two() {
    let protocols: &[UnvalidatedProtoEntry] = &[
        "Desc=1-10,50 Cons=1,3-7,8".parse().unwrap(),
        "Link=8,12-45 Cons=2-6,8 Desc=9".parse().unwrap(),
    ];

    let listed = ProtoverVote::compute(protocols, &2);
    assert_eq!("Cons=3-6,8 Desc=9", listed.to_string());
}

#[test]
fn protover_compute_vote_handles_duplicated_versions() {
    let protocols: &[UnvalidatedProtoEntry] =
        &["Cons=1".parse().unwrap(), "Cons=1".parse().unwrap()];
    assert_eq!("Cons=1", ProtoverVote::compute(protocols, &2).to_string());

    let protocols: &[UnvalidatedProtoEntry] =
        &["Cons=1-2".parse().unwrap(), "Cons=1-2".parse().unwrap()];
    assert_eq!("Cons=1-2", ProtoverVote::compute(protocols, &2).to_string());
}

#[test]
fn protover_compute_vote_handles_invalid_proto_entries() {
    let protocols: &[UnvalidatedProtoEntry] = &[
        "Cons=1".parse().unwrap(),
        "Cons=1".parse().unwrap(),
        "Dinosaur=1".parse().unwrap(),
    ];
    assert_eq!("Cons=1", ProtoverVote::compute(protocols, &2).to_string());
}

#[test]
fn parse_protocol_with_single_protocol_and_two_nonsequential_versions() {
    let _: ProtoEntry = "Desc=1,2".parse().unwrap();
}

#[test]
fn protover_is_supported_here_returns_true_for_supported_protocol() {
    assert_eq!(
        true,
        protover::is_supported_here(&protover::Protocol::Cons, &1)
    );
}

#[test]
fn protover_is_supported_here_returns_false_for_unsupported_protocol() {
    assert_eq!(
        false,
        protover::is_supported_here(&protover::Protocol::Cons, &5)
    );
}

#[test]
fn protocol_all_supported_with_single_proto_and_single_version() {
    let protocol: UnvalidatedProtoEntry = "Cons=1".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocol.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protocol_all_supported_with_single_protocol_and_multiple_versions() {
    let protocol: UnvalidatedProtoEntry = "Cons=1-2".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocol.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protocol_all_supported_with_different_single_protocol_and_single_version() {
    let protocol: UnvalidatedProtoEntry = "HSDir=1".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocol.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protocol_all_supported_with_single_protocol_and_supported_version() {
    let protocol: UnvalidatedProtoEntry = "Desc=2".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocol.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protocol_all_supported_with_two_protocols_and_single_version() {
    let protocols: UnvalidatedProtoEntry = "Cons=1 HSDir=1".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocols.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protocol_all_supported_with_single_protocol_and_two_nonsequential_versions() {
    let protocol: UnvalidatedProtoEntry = "Desc=1,2".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocol.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protocol_all_supported_with_single_protocol_and_two_sequential_versions() {
    let protocol: UnvalidatedProtoEntry = "Desc=1-2".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocol.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protocol_all_supported_with_single_protocol_and_protocol_range() {
    let protocol: UnvalidatedProtoEntry = "Link=1-4".parse().unwrap();
    let unsupported: Option<UnvalidatedProtoEntry> = protocol.all_supported();
    assert_eq!(true, unsupported.is_none());
}

#[test]
fn protover_all_supported_should_exclude_versions_we_actually_do_support() {
    let proto: UnvalidatedProtoEntry = "Link=3-63".parse().unwrap();
    let result: String = proto.all_supported().unwrap().to_string();

    assert_eq!(result, "Link=6-63".to_string());
}

#[test]
fn protover_all_supported_should_exclude_versions_we_actually_do_support_complex1() {
    let proto: UnvalidatedProtoEntry = "Link=1-3,30-63".parse().unwrap();
    let result: String = proto.all_supported().unwrap().to_string();

    assert_eq!(result, "Link=30-63".to_string());
}

#[test]
fn protover_all_supported_should_exclude_versions_we_actually_do_support_complex2() {
    let proto: UnvalidatedProtoEntry = "Link=1-3,5-12".parse().unwrap();
    let result: String = proto.all_supported().unwrap().to_string();

    assert_eq!(result, "Link=6-12".to_string());
}

#[test]
fn protover_all_supported_should_exclude_some_versions_and_entire_protocols() {
    let proto: UnvalidatedProtoEntry = "Link=1-3,5-12 Quokka=50-51".parse().unwrap();
    let result: String = proto.all_supported().unwrap().to_string();

    assert_eq!(result, "Link=6-12 Quokka=50-51".to_string());
}

#[test]
// C_RUST_DIFFERS: The C will return true (e.g. saying "yes, that's supported")
// but set the msg to NULL (??? seems maybe potentially bad).  The Rust will
// simply return a None.
fn protover_all_supported_should_return_empty_string_for_weird_thing() {
    let proto: UnvalidatedProtoEntry = "Fribble=".parse().unwrap();
    let result: Option<UnvalidatedProtoEntry> = proto.all_supported();

    assert!(result.is_none());
}

#[test]
fn protover_unvalidatedprotoentry_should_err_entirely_unparseable_things() {
    let proto: Result<UnvalidatedProtoEntry, ProtoverError> = "Fribble".parse();

    assert_eq!(Err(ProtoverError::Unparseable), proto);
}

#[test]
fn protover_all_supported_over_maximum_limit() {
    let proto: Result<UnvalidatedProtoEntry, ProtoverError> = "Sleen=1-4294967295".parse();

    assert_eq!(Err(ProtoverError::ExceedsMax), proto);
}
