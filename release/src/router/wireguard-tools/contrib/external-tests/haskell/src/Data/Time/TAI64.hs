module Data.Time.TAI64 (
    TAI64(..)
  , TAI64N(..)
  , TAI64NA(..)
  , posixToTAI64
  , posixToTAI64N
  , posixToTAI64NA
  , getCurrentTAI64
  , getCurrentTAI64N
  , getCurrentTAI64NA
  , tAI64ToPosix
  , tAI64NToPosix
  , tAI64NAToPosix
) where

import Data.Serialize
import Control.Monad
import Data.Word

import Data.Time.Clock
import Data.Time.Clock.POSIX

import Numeric

data TAI64 = TAI64
  {-# UNPACK #-} !Word64
  deriving (Eq, Ord)

data TAI64N = TAI64N
  {-# UNPACK #-} !TAI64
  {-# UNPACK #-} !Word32
  deriving (Eq, Ord, Show)

data TAI64NA = TAI64NA
  {-# UNPACK #-} !TAI64N
  {-# UNPACK #-} !Word32
  deriving (Eq, Ord, Show)

instance Show TAI64   where
  show (TAI64 t) = "TAI64 0x" ++ showHex t ""

instance Serialize TAI64 where
  put (TAI64 t) = putWord64be t
  get = liftM TAI64 get

instance Serialize TAI64N where
  put (TAI64N  t' nt) = put t' >> putWord32be nt
  get = liftM2 TAI64N  get get

instance Serialize TAI64NA where
  put (TAI64NA t' at) = put t' >> putWord32be at
  get = liftM2 TAI64NA get get


posixToTAI64 :: POSIXTime -> TAI64
posixToTAI64 = TAI64 . (2^62 +) . truncate . realToFrac

posixToTAI64N :: POSIXTime -> TAI64N
posixToTAI64N pt = TAI64N t' ns where
  t' = posixToTAI64 pt
  ns = (`mod` 10^9) $ truncate (pts * 10**9)
  pts = realToFrac pt

posixToTAI64NA :: POSIXTime -> TAI64NA -- | PICOsecond precision
posixToTAI64NA pt = TAI64NA t' as where
  t' = posixToTAI64N pt
  as = (`mod` 10^9) $ truncate (pts * 10**18)
  pts = realToFrac pt

getCurrentTAI64   :: IO TAI64
getCurrentTAI64N  :: IO TAI64N
getCurrentTAI64NA :: IO TAI64NA
getCurrentTAI64   = liftM posixToTAI64   getPOSIXTime
getCurrentTAI64N  = liftM posixToTAI64N  getPOSIXTime
getCurrentTAI64NA = liftM posixToTAI64NA getPOSIXTime

tAI64ToPosix :: TAI64 -> POSIXTime
tAI64ToPosix (TAI64 s) = fromRational . fromIntegral $ s - 2^62

tAI64NToPosix :: TAI64N -> POSIXTime
tAI64NToPosix (TAI64N t' n) = tAI64ToPosix t' + nanopart where
  nanopart = fromRational $ (toRational $ 10**(-9)) * toRational n -- TODO: optimize?

tAI64NAToPosix :: TAI64NA -> POSIXTime
tAI64NAToPosix (TAI64NA t' a) = tAI64NToPosix t' + attopart where
  attopart = fromRational $ (toRational $ 10**(-18)) * toRational a
