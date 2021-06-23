shopt -s globstar
for f in ./**/*MSP430*; do
mv $f $(echo $f | sed 's/MSP430/Y86/g');
done

# cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=Y86 -G "Unix Makefiles" -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=Y86 ../llvm