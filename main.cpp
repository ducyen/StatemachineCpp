#include <iostream>
#include <cassert>

/*-------1---------2---------3---------4---------5---------6---------7---------8---------9--------*/

class Context;
union EventParams;
enum EventId {
    E0,
    E1,
    E2,
    E3,
    E4,
    E5,
};

union EventParams {

};

/*------------------------------------------- Common ---------------------------------------------*/
class Common {
public:
    class Statemachine;

    class TopState {
        using TThisState  = TopState;
    public:
        TopState() {}
        static TopState* GetInstance() {
            static TopState singleInstance;
            return &singleInstance;
        }
        virtual void Entry(Context* pContext, Statemachine* pStm) {
            if (pStm->IsEnterable<TThisState>()) {
            }
        }
        virtual void Exit(Context* pContext, Statemachine* pStm) {
            if (pStm->IsExitable<TThisState>()) {
            }
        }
        virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
            return false;
        }
    };

    class Statemachine {
        bool          m_bIsExternTrans;
    protected:
        TopState*     m_pLCAState;
        TopState*     m_pTargetState;
    public:
        TopState*     m_pPseudostate;
        TopState*     m_pCurrentState;
        Statemachine* m_pParentStm;
        TopState*     m_pSourceState;

        TopState      m_TopStateStart;
    public:
        Statemachine():
            m_pParentStm(nullptr),
            m_bIsExternTrans(false),
            m_pCurrentState(TopState::GetInstance()),
            m_pLCAState(TopState::GetInstance()),
            m_pTargetState(TopState::GetInstance()),
            m_pSourceState(TopState::GetInstance()),
            m_pPseudostate(TopState::GetInstance())
        {
        }
        template<class TCompositeState = TopState>
        static bool IsIn(TopState* pLeafState) {
            bool bResult = dynamic_cast<TCompositeState*>(pLeafState) != nullptr;
            return bResult;
        }
        template<class TThisState = TopState>
        bool IsEnterable() {
            bool isThisLCA = IsIn<TThisState>(m_pLCAState);
            if (!isThisLCA) {
                return true;
            }
            return false;
        }
        template<class TThisState = TopState>
        bool IsExitable() {
            bool isThisLCA = IsIn<TThisState>(m_pSourceState) && IsIn<TThisState>(m_pTargetState);
            if (!isThisLCA) {
                return true;
            } else if (m_bIsExternTrans) {
                m_bIsExternTrans = false;
                return true;
            } else { 
                m_pLCAState = TThisState::GetInstance();
            }
            return false;
        }
        bool IsFinished() {
            return m_pCurrentState == TopState::GetInstance() && m_pCurrentState == m_pPseudostate;
        }
        void EndTrans(Context* pContext) {
            m_pCurrentState = m_pTargetState;
            m_pCurrentState->Entry(pContext, this);
        }
        void BgnTrans(Context* pContext, TopState* pTargetState) {
            BgnTrans(pContext, pTargetState, pTargetState, false);
        }
        void BgnTrans(Context* pContext, TopState* pTargetState, TopState* pPseudostate) {
            BgnTrans(pContext, pTargetState, pPseudostate, false);
        }
        void BgnTrans(Context* pContext, TopState* pTargetState, bool bIsExternTrans) {
            BgnTrans(pContext, pTargetState, pTargetState, bIsExternTrans);
        }
        void BgnTrans(Context* pContext, TopState* pTargetState, TopState* pPseudostate, bool bIsExternTrans) {
            m_pTargetState = pTargetState;
            m_pPseudostate = pPseudostate;
            m_bIsExternTrans = bIsExternTrans;
            m_pCurrentState->Exit(pContext, this);
        }
        virtual bool DefaultTrans(Context* pContext) = 0;
        virtual bool EventProc(Context* pContext, EventId nEventId, EventParams* pParams) = 0;
        bool RunToCompletion(Context* pContext) {
            bool bResult;
            do {
                m_pSourceState = m_pCurrentState;
                m_pLCAState = TopState::GetInstance();
                bResult = DefaultTrans(pContext);
            } while (bResult);
            return bResult;
        }
        bool Reset(Context* pContext, Statemachine* pParentStm, TopState* pEntryPoint) {
            if (!IsFinished()) {
                std::cout << " entered by entry-point, not initial-point\n";
                return false;
            }
            if (pEntryPoint == nullptr) {
                m_pPseudostate = &m_TopStateStart;
            } else {
                m_pPseudostate = pEntryPoint;
            }
            m_pParentStm = pParentStm;
            return RunToCompletion(pContext);
        }
        bool Abort(Context* pContext) {
            m_pSourceState = TopState::GetInstance();
            BgnTrans(pContext, TopState::GetInstance());
            EndTrans(pContext);
            return true;
        }
    };
};

/*------------------------------------------ Specific --------------------------------------------*/
class Context {

    using Statemachine = Common::Statemachine;
    using TopState = Common::TopState;

    class S18Stm: public Statemachine {
    public:

        TopState m_S18Entry1;
        /*------------------------------------------ S181 --------------------------------------------*/
        class S181: public TopState {
            using TThisState  = S181;
            using TSuperState = TopState;
        protected:
            S181() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    /* entry */
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E0:
                    if (std::cout << "g1\n") {
                        pStm->BgnTrans(pContext, S182::GetInstance());
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*------------------------------------------ S182 --------------------------------------------*/
        class S182: public TopState {
            using TThisState  = S182;
            using TSuperState = TopState;
        protected:
            S182() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    /* entry */
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E2:
                    if (std::cout << "g1\n") {
                        pStm->BgnTrans(pContext, TopState::GetInstance());
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                case E3:
                    if (std::cout << "g1\n") {
                        pStm->BgnTrans(pContext, TopState::GetInstance());
                        pStm->m_pParentStm->m_pPseudostate = &((MainStm*)pStm->m_pParentStm)->m_S18Exit1;
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };
        /*-------------------------------------- DefaultTrans ----------------------------------------*/
        virtual bool DefaultTrans(Context* pContext) {
            bool bResult = false;
            /* substms' run-to-completion */
            if (m_pPseudostate == &m_TopStateStart) {
                BgnTrans(pContext, S181::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else if (m_pPseudostate == &m_S18Entry1) {
                BgnTrans(pContext, S181::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else {
                /* entry-points */
                /* final-points */
            }
            return bResult;
        }

    public:
        virtual bool EventProc(Context* pContext, EventId nEventId, EventParams* pParams) {
            bool bResult = false;
            m_pLCAState = TopState::GetInstance();
            /* substms' event-proc */
            bResult = m_pCurrentState->EventProc(pContext, this, nEventId, pParams);
            RunToCompletion(pContext);
            return bResult;
        }
    };

    class S111Stm: public Statemachine {
    public:
        TopState m_S111Entry1;

        /*------------------------------------------ S14 --------------------------------------------*/
        class S14: public TopState {
            using TThisState  = S14;
            using TSuperState = TopState;
        protected:
            S14() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    /* entry */
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E2:
                    if (std::cout << "g1\n") {
                        pStm->BgnTrans(pContext, S15::GetInstance());
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*------------------------------------------ S15 ---------------------------------------------*/
        class S15: public TopState {
            using TThisState  = S15;
            using TSuperState = TopState;
        protected:
            S15() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    /* entry */
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*-------------------------------------- DefaultTrans ----------------------------------------*/
        virtual bool DefaultTrans(Context* pContext) {
            bool bResult = false;
            /* substms' run-to-completion */
            if (m_pPseudostate == &m_TopStateStart) {
                BgnTrans(pContext, S15::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else if (m_pPseudostate == &m_S111Entry1) {
                BgnTrans(pContext, S14::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else {
                /* entry-points */
                /* final-points */
            }
            return bResult;
        }

    public:
        virtual bool EventProc(Context* pContext, EventId nEventId, EventParams* pParams) {
            bool bResult = false;
            m_pLCAState = TopState::GetInstance();
            /* substms' event-proc */
            bResult = m_pCurrentState->EventProc(pContext, this, nEventId, pParams);
            RunToCompletion(pContext);
            return bResult;
        }
    };

    class S112Stm: public Statemachine {
    public:
        TopState m_S112Entry1;
        TopState m_S112Entry2;

        /*------------------------------------------ S14 --------------------------------------------*/
        class S16: public TopState {
            using TThisState  = S16;
            using TSuperState = TopState;
        protected:
            S16() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    /* entry */
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E3:
                    if (std::cout << "g1\n") {
                        pStm->BgnTrans(pContext, S17::GetInstance());
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*------------------------------------------ S15 ---------------------------------------------*/
        class S17: public TopState {
            using TThisState  = S17;
            using TSuperState = TopState;
        protected:
            S17() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    /* entry */
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*-------------------------------------- DefaultTrans ----------------------------------------*/
        virtual bool DefaultTrans(Context* pContext) {
            bool bResult = false;
            /* substms' run-to-completion */
            if (m_pPseudostate == &m_TopStateStart) {
                BgnTrans(pContext, S16::GetInstance());
                std::cout << "s111 init\n";
                EndTrans(pContext);
                bResult = true;
            } else if (m_pPseudostate == &m_S112Entry1) {
                BgnTrans(pContext, S16::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else if (m_pPseudostate == &m_S112Entry2) {
                BgnTrans(pContext, S17::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else {
                /* entry-points */
                /* final-points */
            }
            return bResult;
        }

    public:
        virtual bool EventProc(Context* pContext, EventId nEventId, EventParams* pParams) {
            bool bResult = false;
            m_pLCAState = TopState::GetInstance();
            /* substms' event-proc */
            bResult = m_pCurrentState->EventProc(pContext, this, nEventId, pParams);
            RunToCompletion(pContext);
            return bResult;
        }
    };

    class MainStm: public Statemachine {
    public:
        TopState* m_pS6History;
        TopState* m_pS9History;

        S111Stm   m_S111Stm;
        S112Stm   m_S112Stm;
        S18Stm    m_S18Stm;

        TopState m_S2Start;
        TopState m_S6Start;
        TopState m_S9Start;
        TopState m_S11Start;
        TopState m_Junction;
        TopState m_S18Exit1;

        /*------------------------------------------- S1 ---------------------------------------------*/
        class S1: public TopState {
            using TThisState  = S1;
            using TSuperState = TopState;
        protected:
            S1() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    /* entry */
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E1:
                    if (std::cout << "g1\n") {
                        pStm->BgnTrans(pContext, S2::GetInstance(), &((MainStm*)pStm)->m_S2Start);
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                case E2:
                    if (std::cout << "Enter condition1: ") {
                        char n;
                        std::cin >> n;
                        if (n - '0') {
                            if (((MainStm*)pStm)->m_pS6History) {
                                pStm->BgnTrans(pContext, ((MainStm*)pStm)->m_pS6History);
                                pStm->EndTrans(pContext);
                                bResult = true;
                            } else {
                                pStm->BgnTrans(pContext, S6::GetInstance(), &((MainStm*)pStm)->m_S6Start);
                                pStm->EndTrans(pContext);
                                bResult = true;
                            }
                        } else {
                            std::cout << "Enter condition2: ";
                            std::cin >> n;
                            if (n - '0') {
                                pStm->BgnTrans(pContext, S10::GetInstance());
                                pStm->EndTrans(pContext);
                                bResult = true;
                            } else {
                                pStm->BgnTrans(pContext, S9::GetInstance(), &((MainStm*)pStm)->m_S9Start);
                                pStm->EndTrans(pContext);
                                bResult = true;
                            }
                        }
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*------------------------------------------- S2 ---------------------------------------------*/
        class S2: public TopState {
            using TThisState  = S2;
            using TSuperState = TopState ;
        protected:
            S2() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E3:
                    if (std::cout << "g\n") {
                        if (((MainStm*)pStm)->m_pS6History) {
                            pStm->BgnTrans(pContext, ((MainStm*)pStm)->m_pS6History);
                            pStm->EndTrans(pContext);
                            bResult = true;
                        } else {
                            pStm->BgnTrans(pContext, S6::GetInstance(), &((MainStm*)pStm)->m_S6Start);
                            pStm->EndTrans(pContext);
                            bResult = true;
                        }
                    }
                    break;
                case E2:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S5::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        class S3: public S2 {
            using TThisState  = S3;
            using TSuperState = S2;
        protected:
            S3() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E0:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S4::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                case E1:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S5::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        class S4: public S2 {
            using TThisState  = S4;
            using TSuperState = S2;
        protected:
            S4() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E0:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S2::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*------------------------------------------- S5 ---------------------------------------------*/
        class S5: public TopState {
            using TThisState  = S5;
            using TSuperState = TopState;
        protected:
            S5() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E0:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, &((MainStm*)pStm)->m_Junction);
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*------------------------------------------- S6 ---------------------------------------------*/
        class S6: public TopState {
            using TThisState  = S6;
            using TSuperState = TopState;
        protected:
            S6() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E1:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, &((MainStm*)pStm)->m_Junction);
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                case E2:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S8::GetInstance(), true);
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        class S7: public S6 {
            using TThisState  = S7;
            using TSuperState = S6;
        protected:
            S7() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                    /* history */
                    ((MainStm*)pStm)->m_pS6History = GetInstance();
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E1:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S8::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        class S8: public S6 {
            using TThisState  = S8;
            using TSuperState = S6;
        protected:
            S8() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                    /* history */
                    ((MainStm*)pStm)->m_pS6History = GetInstance();
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*------------------------------------------- S9 ---------------------------------------------*/
        class S9: public TopState {
            using TThisState  = S9;
            using TSuperState = TopState;
        protected:
            S9() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                    /* history */
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E1:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S91::GetInstance(), true);
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                case E2:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S92::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                case E3:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S11::GetInstance(), &((MainStm*)pStm)->m_S11Start);
                        ((MainStm*)pStm)->m_S111Stm.Reset(pContext, pStm, &((MainStm*)pStm)->m_S111Stm.m_S111Entry1);
                        ((MainStm*)pStm)->m_S112Stm.Reset(pContext, pStm, &((MainStm*)pStm)->m_S112Stm.m_S112Entry1);
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        class S91: public S9 {
            using TThisState  = S91;
            using TSuperState = S9;
        protected:
            S91() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                    /* history */
                    ((MainStm*)pStm)->m_pS9History = GetInstance();
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        class S92: public S9 {
            using TThisState  = S92;
            using TSuperState = S9;
        protected:
            S92() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                    /* history */
                    ((MainStm*)pStm)->m_pS9History = GetInstance();
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*------------------------------------------- S10 --------------------------------------------*/
        class S10: public TopState {
            using TThisState  = S10;
            using TSuperState = TopState;
        protected:
            S10() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                    /* history */
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E1:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S10::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                case E2:
                    if (std::cout << "Enter condition: \n") {
                        char n;
                        std::cin >> n;
                        if (n - '0' == 0) {
                            pStm->BgnTrans(pContext, S7::GetInstance());
                            std::cout << "a\n";
                            pStm->EndTrans(pContext);
                            bResult = true;
                        } else if (n - '0' == 1) {
                            pStm->BgnTrans(pContext, S18::GetInstance());
                            std::cout << "a\n";
                            ((MainStm*)pStm)->m_S18Stm.Reset(pContext, pStm, &((MainStm*)pStm)->m_S18Stm.m_S18Entry1);
                            pStm->EndTrans(pContext);
                            bResult = true;
                        } else {
                            pStm->BgnTrans(pContext, S11::GetInstance(), &((MainStm*)pStm)->m_S11Start);
                            std::cout << "a\n";
                            pStm->EndTrans(pContext);
                            bResult = true;
                        }
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*------------------------------------------- S11 --------------------------------------------*/
        class S11: public TopState {
            using TThisState  = S11;
            using TSuperState = TopState;
        protected:
            S11() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    ((MainStm*)pStm)->m_S111Stm.Reset(pContext, pStm, nullptr);
                    ((MainStm*)pStm)->m_S112Stm.Reset(pContext, pStm, nullptr);
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                    /* history */
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                    ((MainStm*)pStm)->m_S111Stm.Abort(pContext);
                    ((MainStm*)pStm)->m_S112Stm.Abort(pContext);
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E4:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, TopState::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                case E5:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, ((MainStm*)pStm)->m_pS9History);
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        class S12: public S11 {
            using TThisState  = S12;
            using TSuperState = S11;
        protected:
            S12() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                    /* history */
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E1:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S13::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        class S13: public S11 {
            using TThisState  = S13;
            using TSuperState = S11;
        protected:
            S13() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                    /* history */
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E5:
                    if (IsIn<S111Stm::S15>(((MainStm*)pStm)->m_S111Stm.m_pCurrentState)) {
                        pStm->BgnTrans(pContext, S20::GetInstance());
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };


        /*------------------------------------------- S18 --------------------------------------------*/
        class S18: public TopState {
            using TThisState  = S18;
            using TSuperState = TopState;
        protected:
            S18() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    ((MainStm*)pStm)->m_S18Stm.Reset(pContext, pStm, nullptr);
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                    ((MainStm*)pStm)->m_S18Stm.Abort(pContext);
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E3:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S19::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                case E4:
                    if (std::cout << "g\n") {
                        if (((MainStm*)pStm)->m_pS6History) {
                            pStm->BgnTrans(pContext, ((MainStm*)pStm)->m_pS6History);
                            pStm->EndTrans(pContext);
                            bResult = true;
                        } else {
                            pStm->BgnTrans(pContext, S6::GetInstance(), &((MainStm*)pStm)->m_S6Start);
                            pStm->EndTrans(pContext);
                            bResult = true;
                        }
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*------------------------------------------- S19 --------------------------------------------*/
        class S19: public TopState {
            using TThisState  = S19;
            using TSuperState = TopState;
        protected:
            S19() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E0:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S20::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*------------------------------------------- S20 --------------------------------------------*/
        class S20: public TopState {
            using TThisState  = S20;
            using TSuperState = TopState;
        protected:
            S20() {}
        public:
            static TopState* GetInstance() {
                static TThisState singleInstance;
                return &singleInstance;
            }
            virtual void Entry(Context* pContext, Statemachine* pStm) {
                if (pStm->IsEnterable<TThisState>()) {
                    /* substms' reset */
                    /* entry */
                    TSuperState::GetInstance()->Entry(pContext, pStm);
                    std::cout << typeid(this).name() << " entry\n";
                }
            }
            virtual void Exit(Context* pContext, Statemachine* pStm) {
                if (pStm->IsExitable<TThisState>()) {
                    /* exit */
                    std::cout << typeid(this).name() << " exit\n";
                    TSuperState::GetInstance()->Exit(pContext, pStm);
                    /* substms' abort */
                }
            }
            virtual bool EventProc(Context* pContext, Statemachine* pStm, EventId nEventId, EventParams* pParams) {
                bool bResult = false;
                pStm->m_pSourceState = TThisState::GetInstance();
                switch (nEventId) {
                case E1:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, TopState::GetInstance());
                        std::cout << "a\n";
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                case E2:
                    if (std::cout << "g\n") {
                        pStm->BgnTrans(pContext, S11::GetInstance(), &((MainStm*)pStm)->m_S11Start);
                        std::cout << "a\n";
                        ((MainStm*)pStm)->m_S112Stm.Reset(pContext, pStm, &((MainStm*)pStm)->m_S112Stm.m_S112Entry2);
                        pStm->EndTrans(pContext);
                        bResult = true;
                    }
                    break;
                default:
                    break;
                }
                return bResult ? bResult : TSuperState::GetInstance()->EventProc(pContext, pStm, nEventId, pParams);
            }
        };

        /*-------------------------------------- DefaultTrans ----------------------------------------*/
        virtual bool DefaultTrans(Context* pContext) {
            bool bResult = false;
            /* substms' run-to-completion */
            if (m_pPseudostate == &m_TopStateStart) {
                BgnTrans(pContext, S1::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else if (m_pPseudostate == &m_S2Start) {
                BgnTrans(pContext, S3::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else if (m_pPseudostate == &m_S6Start) {
                BgnTrans(pContext, S7::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else if (m_pPseudostate == &m_S9Start) {
                BgnTrans(pContext, S91::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else if (m_pPseudostate == &m_S11Start) {
                BgnTrans(pContext, S12::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else if (m_pPseudostate == &m_S18Exit1) {
                BgnTrans(pContext, S19::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else if (m_pPseudostate == &m_Junction) {
                BgnTrans(pContext, S18::GetInstance());
                std::cout << typeid(m_pPseudostate).name() << " init\n";
                EndTrans(pContext);
                bResult = true;
            } else {
                /* entry-points */
                /* final-points */
            }
            return bResult;
        }

    public:
        virtual bool EventProc(Context* pContext, EventId nEventId, EventParams* pParams) {
            bool bResult = false;
            m_pLCAState = TopState::GetInstance();
            /* substms' event-proc */
            bResult |= m_S111Stm.EventProc(pContext, nEventId, pParams);
            bResult |= m_S112Stm.EventProc(pContext, nEventId, pParams);
            bResult |= m_S18Stm.EventProc(pContext, nEventId, pParams);

            /* this stm's event-proc */
            bResult |= m_pCurrentState->EventProc(pContext, this, nEventId, pParams);
            RunToCompletion(pContext);
            return bResult;
        }
    };

    MainStm mainStm;
public:
    bool Start() {
        mainStm.Abort(this);
        return mainStm.Reset(this, nullptr, nullptr);
    }
    bool EventProc(EventId nEventId, EventParams* pParams) {
        return mainStm.EventProc(this, nEventId, pParams);
    }
};

/*---------------------------------------------------------------------------------------------*/

int main() {
    std::cout << "Hello World!\n";
    Context ctxTest;
    char n;
    do {
        ctxTest.Start();
        do {
            std::cout << "Enter event number('q': quit, 'r':restart): E";
            std::cin  >> n;
            ctxTest.EventProc((EventId)(n - '0'), nullptr);
        } while (n != 'q' && n != 'r');
    }while (n != 'q');
}
