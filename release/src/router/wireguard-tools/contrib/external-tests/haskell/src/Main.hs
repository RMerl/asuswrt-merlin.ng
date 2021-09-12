module Main where

import           Control.Monad              (void)
import           Crypto.Hash.BLAKE2.BLAKE2s (hash)
import           Data.ByteArray             (ScrubbedBytes, convert)
import           Data.ByteString            (ByteString, replicate, take, drop)
import qualified Data.ByteString.Base16     as B16
import qualified Data.ByteString.Base64     as B64
import           Data.Maybe                 (fromMaybe)
import           Data.Monoid                ((<>))
import qualified Data.Serialize             as S
import           Network.Socket
import qualified Network.Socket.ByteString  as NBS
import           Prelude hiding             (replicate, take, drop)

import Crypto.Noise
import Crypto.Noise.Cipher
import Crypto.Noise.Cipher.ChaChaPoly1305
import Crypto.Noise.DH
import Crypto.Noise.DH.Curve25519
import Crypto.Noise.HandshakePatterns (noiseIKpsk2)
import Crypto.Noise.Hash hiding (hash)
import Crypto.Noise.Hash.BLAKE2s

import Data.Time.TAI64

sampleICMPRequest :: ByteString
sampleICMPRequest = fst . B16.decode $
  "450000250000000014018f5b0abd81020abd810108001bfa039901b6576972654775617264"

validateICMPResponse :: ByteString
                     -> Bool
validateICMPResponse r =
  -- Strip off part of IPv4 header because this is only a demo.
  drop 12 sample == drop 12 r
  where
    sample = fst . B16.decode $ "45000025e3030000400180570abd81010abd8102000023fa039901b65769726547756172640000000000000000000000"

unsafeMessage :: (Cipher c, DH d, Hash h)
              => Bool
              -> Maybe ScrubbedBytes
              -> ScrubbedBytes
              -> NoiseState c d h
              -> (ScrubbedBytes, NoiseState c d h)
unsafeMessage write mpsk msg ns = case operation msg ns of
  NoiseResultMessage ct ns' -> (ct, ns')

  NoiseResultNeedPSK ns' -> case mpsk of
    Nothing -> error "psk required but not provided"
    Just k  -> case operation k ns' of
      NoiseResultMessage ct ns'' -> (ct, ns'')
      _ -> error "something terrible happened"

  _ -> error "something terrible happened"
  where
    operation = if write then writeMessage else readMessage

main :: IO ()
main = do
  let ip           = "demo.wireguard.com"
      port         = "12913"
      myKeyB64     = "WAmgVYXkbT2bCtdcDwolI88/iVi/aV3/PHcUBTQSYmo=" -- private key
      serverKeyB64 = "qRCwZSKInrMAq5sepfCdaCsRJaoLe5jhtzfiw7CjbwM=" -- public key
      pskB64       = "FpCyhws9cxwWoV4xELtfJvjJN+zQVRPISllRWgeopVE="

  addrInfo <- head <$> getAddrInfo Nothing (Just ip) (Just port)
  sock     <- socket (addrFamily addrInfo) Datagram defaultProtocol

  let addr        = addrAddress addrInfo
      myStaticKey = fromMaybe (error "invalid private key")
                    . dhBytesToPair
                    . convert
                    . either (error "error Base64 decoding my private key") id
                    . B64.decode
                    $ myKeyB64 :: KeyPair Curve25519

      serverKey   = fromMaybe (error "invalid public key")
                    . dhBytesToPub
                    . convert
                    . either (error "error Base64 decoding server public key") id
                    . B64.decode
                    $ serverKeyB64 :: PublicKey Curve25519

      psk         = convert
                    . either (error "error decoding PSK") id
                    . B64.decode
                    $ pskB64 :: ScrubbedBytes

  myEphemeralKey <- dhGenKey

  let dho  = defaultHandshakeOpts InitiatorRole "WireGuard v1 zx2c4 Jason@zx2c4.com"
      opts = setLocalEphemeral (Just myEphemeralKey)
             . setLocalStatic  (Just myStaticKey)
             . setRemoteStatic (Just serverKey)
             $ dho
      ns0  = noiseState opts noiseIKpsk2 :: NoiseState ChaChaPoly1305 Curve25519 BLAKE2s

  tai64n <- convert . S.encode <$> getCurrentTAI64N

  -- Handshake: Initiator to responder -----------------------------------------

  let (msg0, ns1) = unsafeMessage True Nothing tai64n ns0
      macKey      = hash 32 mempty $ "mac1----" `mappend` (convert . dhPubToBytes) serverKey
      initiation  = "\x01\x00\x00\x00\x1c\x00\x00\x00" <> convert msg0 -- sender index = 28 to match other examples
      mac1        = hash 16 macKey initiation

  void $ NBS.sendTo sock (initiation <> mac1 <> replicate 16 0) addr

  -- Handshake: Responder to initiator -----------------------------------------

  (response0, _) <- NBS.recvFrom sock 1024

  let theirIndex  = take 4  . drop 4  $ response0
      (_, ns2)    = unsafeMessage False (Just psk) (convert . take 48 . drop 12 $ response0) ns1

  -- ICMP: Initiator to responder ----------------------------------------------

  let (msg1, ns3) = unsafeMessage True Nothing (convert sampleICMPRequest) ns2
      icmp        = "\x04\x00\x00\x00" <> theirIndex <> replicate 8 0 <> convert msg1

  void $ NBS.sendTo sock icmp addr

  -- ICMP: Responder to initiator ----------------------------------------------

  (response1, _) <- NBS.recvFrom sock 1024

  let (icmpPayload, ns4) = unsafeMessage False Nothing (convert . drop 16 $ response1) ns3

  -- KeepAlive: Initiator to responder -----------------------------------------

  if validateICMPResponse . convert $ icmpPayload
    then do
      let (msg2, _) = unsafeMessage True Nothing mempty ns4
          keepAlive = "\x04\x00\x00\x00" <> theirIndex <> "\x01" <> replicate 7 0 <> convert msg2

      void $ NBS.sendTo sock keepAlive addr

    else error "unexpected ICMP response from server!"
