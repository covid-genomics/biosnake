add_library(alp OBJECT
        sls_basic.hpp
        njn_dynprogprob.cpp
        njn_random.cpp
        sls_pvalues.hpp
        njn_random.hpp
        njn_integer.hpp
        njn_doubletype.hpp
        njn_dynprogprob.hpp
        njn_dynprogproblim.hpp
        sls_alp_sim.hpp
        njn_dynprogprobproto.cpp
        njn_localmaxstatutil.hpp
        njn_localmaxstat.cpp
        njn_localmaxstat.hpp
        njn_matrix.hpp
        njn_localmaxstatmatrix.cpp
        sls_alp_regression.hpp
        sls_alp_sim.cpp
        sls_pvalues.cpp
        njn_root.hpp
        njn_dynprogproblim.cpp
        sls_alignment_evaluer.cpp
        njn_localmaxstatmatrix.hpp
        sls_alp_regression.cpp
        njn_dynprogprobproto.hpp
        sls_alp.cpp
        njn_approx.hpp
        njn_memutil.hpp
        njn_localmaxstatutil.cpp
        njn_function.hpp
        njn_ioutil.hpp
        sls_basic.cpp
        njn_uniform.hpp
        sls_alignment_evaluer.hpp
        njn_vector.hpp
        sls_alp_data.cpp
        sls_alp_data.hpp
        njn_ioutil.cpp
        sls_alp.hpp
        sls_normal_distr_array.hpp
        )
set_target_properties(alp PROPERTIES COMPILE_FLAGS "${MMSEQS_CXX_FLAGS} -w" LINK_FLAGS "${MMSEQS_CXX_FLAGS} -w")
