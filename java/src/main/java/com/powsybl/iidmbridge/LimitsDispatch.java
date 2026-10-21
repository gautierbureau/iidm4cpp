package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.*;

import java.util.Collection;
import java.util.List;
import java.util.Optional;

/**
 * The flows limits of a holder, for {@link PropertyDispatcher} and {@link Creation}: a powsybl
 * {@link FlowsLimitsHolder} (3WT leg, boundary line), or one side of a branch, which powsybl has no
 * object for: a {@link BranchSide} (getRelatedByIndex(branch, FLOWS_LIMITS_HOLDER, side)). Both are
 * used through {@link Holder}, so that the dispatcher has one case per operation.
 */
final class LimitsDispatch {

    private LimitsDispatch() {}

    /** The operations of a holder of flows limits that the bridge uses. */
    interface Holder {
        Collection<OperationalLimitsGroup> getOperationalLimitsGroups();

        Optional<String> getSelectedOperationalLimitsGroupId();

        Collection<String> getAllSelectedOperationalLimitsGroupIds();

        List<String> getAllSelectedOperationalLimitsGroupIdsOrdered();

        Optional<OperationalLimitsGroup> getOperationalLimitsGroup(String id);

        Optional<OperationalLimitsGroup> getSelectedOperationalLimitsGroup();

        List<OperationalLimitsGroup> getAllSelectedOperationalLimitsGroups();

        OperationalLimitsGroup newOperationalLimitsGroup(String id);

        void setSelectedOperationalLimitsGroup(String id);

        void addSelectedOperationalLimitsGroup(String id);

        void deselectOperationalLimitsGroup(String id);

        void removeOperationalLimitsGroup(String id);

        void cancelSelectedOperationalLimitsGroup();

        OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup();

        OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup(String id);

        /** The limits of a type of the selected group. */
        Optional<? extends LoadingLimits> getLimits(LimitType type);

        /** The limits of a type of every selected group. */
        Collection<? extends LoadingLimits> getAllSelectedLimits(LimitType type);

        /** The limits of a type of a group. */
        default Optional<? extends LoadingLimits> getLimitsFromId(LimitType type, String id) {
            return getOperationalLimitsGroup(id).flatMap(group -> group.getLoadingLimits(type));
        }
    }

    /** One side of a branch, as a holder of flows limits. */
    record BranchSide(Branch<?> branch, TwoSides side) implements Holder {

        private boolean one() {
            return side == TwoSides.ONE;
        }

        @Override
        public Collection<OperationalLimitsGroup> getOperationalLimitsGroups() {
            return one() ? branch.getOperationalLimitsGroups1() : branch.getOperationalLimitsGroups2();
        }

        @Override
        public Optional<String> getSelectedOperationalLimitsGroupId() {
            return one() ? branch.getSelectedOperationalLimitsGroupId1() : branch.getSelectedOperationalLimitsGroupId2();
        }

        @Override
        public Collection<String> getAllSelectedOperationalLimitsGroupIds() {
            return branch.getAllSelectedOperationalLimitsGroupIds(side);
        }

        @Override
        public List<String> getAllSelectedOperationalLimitsGroupIdsOrdered() {
            return branch.getAllSelectedOperationalLimitsGroupIdsOrdered(side);
        }

        @Override
        public Optional<OperationalLimitsGroup> getOperationalLimitsGroup(String id) {
            return one() ? branch.getOperationalLimitsGroup1(id) : branch.getOperationalLimitsGroup2(id);
        }

        @Override
        public Optional<OperationalLimitsGroup> getSelectedOperationalLimitsGroup() {
            return one() ? branch.getSelectedOperationalLimitsGroup1() : branch.getSelectedOperationalLimitsGroup2();
        }

        @Override
        public List<OperationalLimitsGroup> getAllSelectedOperationalLimitsGroups() {
            return branch.getAllSelectedOperationalLimitsGroups(side);
        }

        @Override
        public OperationalLimitsGroup newOperationalLimitsGroup(String id) {
            return one() ? branch.newOperationalLimitsGroup1(id) : branch.newOperationalLimitsGroup2(id);
        }

        @Override
        public void setSelectedOperationalLimitsGroup(String id) {
            if (one()) {
                branch.setSelectedOperationalLimitsGroup1(id);
            } else {
                branch.setSelectedOperationalLimitsGroup2(id);
            }
        }

        @Override
        public void addSelectedOperationalLimitsGroup(String id) {
            branch.addSelectedOperationalLimitsGroups(side, id);
        }

        @Override
        public void deselectOperationalLimitsGroup(String id) {
            branch.deselectOperationalLimitsGroups(side, id);
        }

        @Override
        public void removeOperationalLimitsGroup(String id) {
            if (one()) {
                branch.removeOperationalLimitsGroup1(id);
            } else {
                branch.removeOperationalLimitsGroup2(id);
            }
        }

        @Override
        public void cancelSelectedOperationalLimitsGroup() {
            if (one()) {
                branch.cancelSelectedOperationalLimitsGroup1();
            } else {
                branch.cancelSelectedOperationalLimitsGroup2();
            }
        }

        @Override
        public OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup() {
            return one() ? branch.getOrCreateSelectedOperationalLimitsGroup1() : branch.getOrCreateSelectedOperationalLimitsGroup2();
        }

        @Override
        public OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup(String id) {
            return one() ? branch.getOrCreateSelectedOperationalLimitsGroup1(id) : branch.getOrCreateSelectedOperationalLimitsGroup2(id);
        }

        @Override
        public Optional<? extends LoadingLimits> getLimits(LimitType type) {
            return branch.getLimits(type, side);
        }

        @Override
        public Collection<? extends LoadingLimits> getAllSelectedLimits(LimitType type) {
            return branch.getAllSelectedLimits(type, side);
        }
    }

    /** A powsybl FlowsLimitsHolder (3WT leg, boundary line). */
    private record FlowsHolder(FlowsLimitsHolder holder) implements Holder {

        @Override
        public Collection<OperationalLimitsGroup> getOperationalLimitsGroups() {
            return holder.getOperationalLimitsGroups();
        }

        @Override
        public Optional<String> getSelectedOperationalLimitsGroupId() {
            return holder.getSelectedOperationalLimitsGroupId();
        }

        @Override
        public Collection<String> getAllSelectedOperationalLimitsGroupIds() {
            return holder.getAllSelectedOperationalLimitsGroupIds();
        }

        @Override
        public List<String> getAllSelectedOperationalLimitsGroupIdsOrdered() {
            return holder.getAllSelectedOperationalLimitsGroupIdsOrdered();
        }

        @Override
        public Optional<OperationalLimitsGroup> getOperationalLimitsGroup(String id) {
            return holder.getOperationalLimitsGroup(id);
        }

        @Override
        public Optional<OperationalLimitsGroup> getSelectedOperationalLimitsGroup() {
            return holder.getSelectedOperationalLimitsGroup();
        }

        @Override
        public List<OperationalLimitsGroup> getAllSelectedOperationalLimitsGroups() {
            return holder.getAllSelectedOperationalLimitsGroups();
        }

        @Override
        public OperationalLimitsGroup newOperationalLimitsGroup(String id) {
            return holder.newOperationalLimitsGroup(id);
        }

        @Override
        public void setSelectedOperationalLimitsGroup(String id) {
            holder.setSelectedOperationalLimitsGroup(id);
        }

        @Override
        public void addSelectedOperationalLimitsGroup(String id) {
            holder.addSelectedOperationalLimitsGroups(id);
        }

        @Override
        public void deselectOperationalLimitsGroup(String id) {
            holder.deselectOperationalLimitsGroups(id);
        }

        @Override
        public void removeOperationalLimitsGroup(String id) {
            holder.removeOperationalLimitsGroup(id);
        }

        @Override
        public void cancelSelectedOperationalLimitsGroup() {
            holder.cancelSelectedOperationalLimitsGroup();
        }

        @Override
        public OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup() {
            return holder.getOrCreateSelectedOperationalLimitsGroup();
        }

        @Override
        public OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup(String id) {
            return holder.getOrCreateSelectedOperationalLimitsGroup(id);
        }

        @Override
        public Optional<? extends LoadingLimits> getLimits(LimitType type) {
            return switch (type) {
                case CURRENT -> holder.getCurrentLimits();
                case ACTIVE_POWER -> holder.getActivePowerLimits();
                case APPARENT_POWER -> holder.getApparentPowerLimits();
                default -> throw notLoadingLimits(type);
            };
        }

        @Override
        public Collection<? extends LoadingLimits> getAllSelectedLimits(LimitType type) {
            return switch (type) {
                case CURRENT -> holder.getAllSelectedCurrentLimits();
                case ACTIVE_POWER -> holder.getAllSelectedActivePowerLimits();
                case APPARENT_POWER -> holder.getAllSelectedApparentPowerLimits();
                default -> throw notLoadingLimits(type);
            };
        }
    }

    static Holder holder(Object obj) {
        return obj instanceof BranchSide side ? side : new FlowsHolder((FlowsLimitsHolder) obj);
    }

    static BranchSide branchSide(Object branch, int side) {
        return new BranchSide((Branch<?>) branch, Creation.enumValue(TwoSides.class, side));
    }

    /** The limit type of a type code: CURRENT_LIMITS, ACTIVE_POWER_LIMITS or APPARENT_POWER_LIMITS. */
    static LimitType limitType(int typeCode) {
        return switch (typeCode) {
            case PropertyCodes.CURRENT_LIMITS -> LimitType.CURRENT;
            case PropertyCodes.ACTIVE_POWER_LIMITS -> LimitType.ACTIVE_POWER;
            case PropertyCodes.APPARENT_POWER_LIMITS -> LimitType.APPARENT_POWER;
            default -> throw new BridgeErrors.PropertyNotFoundException("Not a limits type code: " + typeCode);
        };
    }

    /** The limits of a type of a group, or of the selected group of a holder. */
    static LoadingLimits limits(Object groupOrHolder, LimitType type) {
        Optional<? extends LoadingLimits> limits = groupOrHolder instanceof OperationalLimitsGroup group
            ? group.getLoadingLimits(type)
            : holder(groupOrHolder).getLimits(type);
        return limits.orElse(null);
    }

    private static BridgeErrors.PropertyNotFoundException notLoadingLimits(LimitType type) {
        return new BridgeErrors.PropertyNotFoundException("No loading limits of type " + type);
    }
}
