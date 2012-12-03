-- rwthctf2012, fw (hs noob, sorry)

import Data.Char (ord, chr)
import Numeric (showHex, readHex)
import System.IO (isEOF, hSetBuffering, BufferMode(..), stdin, stdout)
import System.Directory (getDirectoryContents, doesFileExist, removeFile)
import System.Exit (exitSuccess)

repl :: [a] -> [a]
repl xs = xs ++ repl xs

perm :: [Int] -> ([Int] -> [Int])
perm p xs = map (\i -> p !! (i-1)) xs

cycl :: [Int] -> Int -> Int
cycl cs x = if not (x `elem` cs) then x else cycl' (repl cs) x
  where cycl' (c:cs) x = if c == x then head cs else cycl' cs x

pcycl :: Int -> [Int] -> [Int]
pcycl n c = map (cycl c) [1..n]

permFromCycls :: Int -> [[Int]] -> ([Int] -> [Int])
permFromCycls n cs = perm $ foldl (flip $ perm . pcycl n) [1..n] cs

-- This is where the magic happens :-)
cube = map (permFromCycls 48) [ -- ULFRBD
  [[ 1, 3, 8, 6], [ 2, 5, 7, 4], [ 9,33,25,17], [10,34,26,18], [11,35,27,19]],
  [[ 9,11,16,14], [10,13,15,12], [ 1,17,41,40], [ 4,20,44,37], [ 6,22,46,35]],
  [[17,19,24,22], [18,21,23,20], [ 6,25,43,16], [ 7,28,42,13], [ 8,30,41,11]],
  [[25,27,32,30], [26,29,31,28], [ 3,38,43,19], [ 5,36,45,21], [ 8,33,48,24]],
  [[33,35,40,38], [34,37,39,36], [ 3, 9,46,32], [ 2,12,47,29], [ 1,14,48,27]],
  [[41,43,48,46], [42,45,47,44], [14,22,30,38], [15,23,31,39], [16,24,32,40]]]
solved = [1..48]

foo :: Integer -> [Integer] -> Integer
foo b = foldr (\i j -> i + b*j) 0

unfoo :: Integer -> Integer -> [Integer]
unfoo b 0 = []
unfoo b n = (n `mod` b) : (unfoo b (n `div` b))

stoi :: Integral a => [a] -> Integer
stoi = foo 48 . map (flip (-) 1) . map fromIntegral

scramble :: [[Int] -> [Int]] -> [Int]
scramble = foldl (\s m -> m s) solved . reverse

scrambleis :: [Int] -> [Int]
scrambleis = scramble . map (\i -> cube !! i)

scramblei :: Integer -> [Int]
scramblei = scrambleis . map fromInteger . unfoo 6

cayley = stoi . scramblei . foo 256 . map fromIntegral . map ord

-- Unbreakable security. Nothing can go wrong. Patent pending.
hash16 = flip showHex "" . cayley


-- Boring boilerplate follows.
-- Sorry for the incredible ugliness.
-- I just started learning me some haskell for great good.

nox :: Eq a => a -> [a] -> [a]
nox x [] = []
nox x (y:ys) = if x == y then nox x ys else y : nox x ys

nodots :: [String] -> [String]
nodots = (nox "..") . (nox ".")

strsan :: String -> String
strsan = nox '/'

file k = "/home/erno/cubes/" ++ (strsan k)

unhex = reverse . map chr . map fromInteger . unfoo 256 . first . readHex
  where first ((a,b):xs) = a
        first [] = 0

put :: [String] -> IO ()
put [] = putStrLn "put where, dude?"
put (x:[]) = putStrLn "put what, dude?"
put (k:v:[]) = do
  writeFile (file k) v
  putStrLn "Okay."
put args = putStrLn "you are doing it wrong."

get :: [String] -> IO ()
get [] = putStrLn "get what, dude?"
get (pre:[]) = do
  let filename = file . hash16 . unhex $ pre
  ex <- doesFileExist filename
  if ex then do
    v <- readFile filename
    putStrLn v
  else putStrLn "Item not found."
get args = putStrLn "you are doing it wrong."

list :: [String] -> IO ()
list args = do
  dir <- getDirectoryContents (file "")
  let cubes = nodots dir
  putStrLn $ show (length cubes) ++ " items in database:"
  mapM putStrLn cubes
  return ()

delete :: [String] -> IO ()
delete [] = putStrLn "delete what, dude?"
delete (pre:[]) = do
  let filename = file . hash16 . unhex $ pre
  ex <- doesFileExist filename
  if ex then removeFile filename else return ()
  putStrLn "Okay."
delete args = putStrLn "you are doing it wrong."

exit :: [String] -> IO ()
exit args = do
  putStrLn "Goodbye."
  exitSuccess

empty :: [String] -> IO ()
empty args = return ()

unknown :: String -> IO ()
unknown cmd = do
  putStrLn $ "Mmhhhh... I like curry! But what is " ++ show cmd ++ "?"
  return ()

cmds = [ ("put", put), ("get", get), ("list", list), ("", empty), ("exit", exit), ("delete", delete) ]

dispatch :: [(String, [String] -> IO ())] -> String -> [String] -> IO ()
dispatch [] cmd args = unknown cmd
dispatch ((name,fn):cs) cmd args = if cmd == name then fn args else dispatch cs cmd args

main = do
 hSetBuffering stdout LineBuffering
 hSetBuffering stdin LineBuffering
 putStrLn "Welcome to Erno's Functional Enterprise Cloud Storage with Lamport Security"
 main'

main' = do
  eof <- isEOF
  if not eof then do
    input <- getLine
    let (cmd:args) = if x == [] then [""] else x where x = words input
    dispatch cmds cmd args
    main'
  else exit []
