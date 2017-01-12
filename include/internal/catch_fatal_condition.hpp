/*
 *  Created by Phil on 21/08/2014
 *  Copyright 2014 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef TWOBLUECUBES_CATCH_FATAL_CONDITION_H_INCLUDED
#define TWOBLUECUBES_CATCH_FATAL_CONDITION_H_INCLUDED


namespace Catch {

    // Report the error condition then exit the process
    inline void reportFatal( std::string const& message, int exitCode ) {
        IContext& context = Catch::getCurrentContext();
        IResultCapture* resultCapture = context.getResultCapture();
        resultCapture->handleFatalErrorCondition( message );
    }

} // namespace Catch

#if defined ( CATCH_PLATFORM_WINDOWS ) /////////////////////////////////////////

namespace Catch {

    struct FatalConditionHandler {
		void reset() {}
	};

} // namespace Catch

#else // Not Windows - assumed to be POSIX compatible //////////////////////////

#include <signal.h>

namespace Catch {

    struct SignalDefs {
        int id;
        const char* name;
        void (*originalHandler)(int);
    };
    extern SignalDefs signalDefs[];
    SignalDefs signalDefs[] = {
            { SIGINT,  "SIGINT - Terminal interrupt signal", CATCH_NULL },
            { SIGILL,  "SIGILL - Illegal instruction signal", CATCH_NULL },
            { SIGFPE,  "SIGFPE - Floating point error signal", CATCH_NULL },
            { SIGSEGV, "SIGSEGV - Segmentation violation signal", CATCH_NULL },
            { SIGTERM, "SIGTERM - Termination request signal", CATCH_NULL },
            { SIGABRT, "SIGABRT - Abort (abnormal termination) signal", CATCH_NULL }
    };

    struct FatalConditionHandler {

        static FatalConditionHandler* m_handler;

        static void handleSignal( int sig ) {
            if( m_handler ) {
                std::string name = "<unknown signal>";
                for (std::size_t i = 0; i < sizeof(signalDefs) / sizeof(SignalDefs); ++i) {
                    SignalDefs &def = signalDefs[i];
                    if (sig == def.id) {
                        name = def.name;
                        signal( def.id, def.originalHandler );
                        raise( sig );
                        break;
                    }
                }
                reportFatal(name, -sig);
            }
        }

        FatalConditionHandler() {
            m_handler = this;
            struct sigaction prevAction;
            for( std::size_t i = 0; i < sizeof(signalDefs)/sizeof(SignalDefs); ++i ) {
                SignalDefs& def = signalDefs[i];
                sigaction(def.id, CATCH_NULL, &prevAction );
                def.originalHandler = prevAction.sa_handler;
                signal(def.id, handleSignal);
            }
        }
        ~FatalConditionHandler() {
            reset();
        }
        void reset() {
            if( m_handler ) {
                for( std::size_t i = 0; i < sizeof(signalDefs)/sizeof(SignalDefs); ++i )
                    signal( signalDefs[i].id, signalDefs[i].originalHandler );
                m_handler = CATCH_NULL;
            }
        }
    };

    FatalConditionHandler* FatalConditionHandler::m_handler = CATCH_NULL;

} // namespace Catch

#endif // not Windows

#endif // TWOBLUECUBES_CATCH_FATAL_CONDITION_H_INCLUDED
